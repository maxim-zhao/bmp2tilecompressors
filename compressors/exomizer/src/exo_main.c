/*
 * Copyright (c) 2002 - 2007 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "log.h"
#include "search.h"
#include "optimal.h"
#include "output.h"
#include "getflag.h"
#include "membuf_io.h"
#include "exo_helper.h"
#include "exo_util.h"
#include "parse.h"
#include "named_buffer.h"
#include "desfx.h"

extern struct membuf sfxdecr[];

#define DEFAULT_OUTFILE "a.out"

static void load_plain_file(const char *name, struct membuf *mb)
{
    int file_len;
    int read_len;
    int offset = 0;
    int len = -1;
    FILE *in;

    in = fopen(name, "rb");
    if(in == NULL)
    {
        char *p = strrchr(name, ',');
        if(p == NULL)
        {
            /* not found and no comma  */
            LOG(LOG_ERROR, ("Error: file not found.\n"));
            exit(-1);
        }
        *p = '\0';
        if(str_to_int(p + 1, &offset))
        {
            LOG(LOG_ERROR, ("Error: invalid value for plain file offset.\n"));
            exit(-1);
        }
        in = fopen(name, "rb");
        if(in == NULL)
        {
            p = strrchr(name, ',');
            len = offset;
            if(len < 0)
            {
                LOG(LOG_ERROR, ("Error, value for plain file "
                                "len must not be negative.\n"));
                exit(-1);
            }
            *p = '\0';
            if(str_to_int(p + 1, &offset))
            {
                LOG(LOG_ERROR,
                    ("Error: invalid value for plain file offset.\n"));
                exit(-1);
            }
            in = fopen(name, "rb");
            if(in == NULL)
            {
                /* really not found */
                LOG(LOG_ERROR, ("Error: file not found.\n"));
                exit(-1);
            }
        }
    }
    /* get the real length of the file and validate the offset*/
    if(fseek(in, 0, SEEK_END))
    {
        LOG(LOG_ERROR, ("Error: can't seek to EOF.\n"));
        fclose(in);
        exit(-1);
    }
    file_len = ftell(in);
    if(offset < 0)
    {
        offset += file_len;
    }
    if(len < 0)
    {
        len = file_len - offset;
    }
    if(fseek(in, offset, SEEK_SET))
    {
        LOG(LOG_ERROR, ("Error: can't seek to offset %d.\n", offset));
        fclose(in);
        exit(-1);
    }
    if(offset + len > file_len)
    {
        LOG(LOG_ERROR, ("Error: can't read %d bytes from offset %d.\n",
                        len, offset));
        fclose(in);
        exit(-1);
    }
    LOG(LOG_VERBOSE, ("Reading %d bytes from offset %d.\n", len, offset));
    do
    {
        char buf[1024];
        int r = 1024 < len? 1024: len;
        read_len = fread(buf, 1, r, in);
        if(read_len < r)
        {
            LOG(LOG_ERROR, ("Error: tried to read %d bytes but got %d.\n",
                            r, read_len));
            fclose(in);
            exit(-1);
        }
        membuf_append(mb, buf, r);
        len -= r;
    }
    while(len > 0);
    fclose(in);
}

static
int
do_load(char *file_name, struct membuf *mem)
{
    struct load_info info[1];
    unsigned char *p;

    membuf_clear(mem);
    membuf_append(mem, NULL, 65536);
    p = membuf_get(mem);

    info->basic_txt_start = -1;

    load_located(file_name, p, info);

    /* move memory to beginning of buffer */
    membuf_truncate(mem, info->end);
    membuf_trim(mem, info->start);

    LOG(LOG_NORMAL, (" crunching from $%04X to $%04X ",
                     info->start, info->end));
    return info->start;
}

struct target_info
{
    int id;
    int sys_token;
    int basic_txt_start;
    int end_of_ram;
    const char *model;
};

static
int
do_loads(int filec, char *filev[], struct membuf *mem,
         int basic_txt_start, int sys_token,
         int *basic_var_startp, int *runp)
{
    int run = -1;
    int min_start = 65537;
    int max_end = -1;
    int basic_code = 0;
    int i;
    unsigned char *p;
    struct load_info info[1];


    membuf_clear(mem);
    membuf_append(mem, NULL, 65536);
    p = membuf_get(mem);

    for (i = 0; i < filec; ++i)
    {
        info->basic_txt_start = basic_txt_start;
        load_located(filev[i], p, info);
        run = info->run;
        if(run != -1 && runp != NULL)
        {
            LOG(LOG_DEBUG, ("Propagating found run address $%04X.\n",
                            info->run));
            *runp = info->run;
        }

        /* do we expect any basic file? */
        if(basic_txt_start >= 0)
        {
            if(info->basic_var_start >= 0)
            {
                basic_code = 1;
                if(basic_var_startp != NULL)
                {
                    *basic_var_startp = info->basic_var_start;
                }
                if(runp != NULL && run == -1)
                {
                    /* only if we didn't get run address from load_located
                     * (run is not -1 if we did) */
                    run = find_sys(p + basic_txt_start, sys_token);
                    *runp = run;
                }
            }
        }

        if (info->start < min_start)
        {
            min_start = info->start;
        }
        if (info->end > max_end)
        {
            max_end = info->end;
        }
        if(info->basic_var_start != -1)
        {
            info->basic_var_start = info->basic_var_start;
        }
        if(info->run != -1)
        {
            info->run = info->run;
        }
    }

    if(basic_txt_start >= 0 && !basic_code && run == -1)
    {
        /* no program loaded to the basic start */
        LOG(LOG_ERROR, ("\nError: nothing loaded at the start of basic "
                        "text address ($%04X).\n",
                        basic_txt_start));
        exit(-1);
    }

    /* if we have a basic code loaded and we are doing a proper basic start
     * (the caller don't expect a sys address so runp is NULL */
    if(basic_code && runp == NULL)
    {
        int valuepos = basic_txt_start - 1;
        /* the byte immediatley preceeding the basic start must be 0
         * for basic to function properly. */
        if(min_start > valuepos)
        {
            /* It not covered by the files to crunch. Since the
             * default fill value is 0 we don't need to set it but we
             * need to include that location in the crunch as well. */
            min_start = valuepos;
        }
        else
        {
            int value = p[valuepos];
            /* it has been covered by at least one file. Let's check
             * if it is zero. */
            if(value != 0)
            {
                /* Hm, its not, danger Will Robinson! */
                LOG(LOG_WARNING,
                    ("Warning, basic will probably not work since the value of"
                     " the location \npreceeding the basic start ($%04X)"
                     " is not 0 but %d.\n", valuepos, value));
            }
        }
    }

    /* move memory to beginning of buffer */
    membuf_truncate(mem, max_end);
    membuf_trim(mem, min_start);

    return min_start;
}

static
void print_command_usage(const char *appl, enum log_level level)
{
    /* done */
    LOG(level,
        ("usage: %s level|mem|sfx|raw|desfx [option]... infile[,<address>]...\n"
         "  see the individual commands for more help.\n",
         appl));
}

static
void print_level_usage(const char *appl, enum log_level level,
                       const char *default_outfile)
{
    /* done */
    LOG(level,
        ("usage: %s level [option]... infile[,<address>]...\n"
         "  The level command generates outfiles that are intended to be decrunched on\n"
         "  the fly while being read.\n", appl));
    LOG(level,
        ("  -f            crunch forward\n"));
    print_crunch_flags(level, default_outfile);
    LOG(level,
        (" All infiles are crunched separately and concatenated in the outfile in the\n"
         " order they are given on the command-line.\n"));
}

static
void print_mem_usage(const char *appl, enum log_level level,
                     const char *default_outfile)
{
    /* done */
    LOG(level,
        ("usage: %s mem [option]... infile[,<address>]...\n"
         "  The mem command generates outfiles that are intended to be decrunched from\n"
         "  memory after being loaded or assembled there.\n", appl));
    LOG(level,
        ("  -l <address>  adds load address to the outfile, using \"none\" as <address>\n"
         "                will skip the load address.\n"));
    LOG(level,
        ("  -f            crunch forward\n"));
    print_crunch_flags(level, default_outfile);
    LOG(level,
        (" All infiles are merged into the outfile. They are loaded in the order\n"
         " they are given on the command-line, from left to right.\n"));
}

static
void print_raw_usage(const char *appl, enum log_level level,
                     const char *default_out_name)
{
    LOG(level, ("usage: %s [option]... infile\n", appl));
    LOG(level,
        ("  -b            crunch/decrunch backwards\n"
         "  -r            write outfile in reverse order\n"
         "  -d            decrunch (instead of crunch)\n"));
    print_crunch_flags(level, default_out_name);
}

static
void print_sfx_usage(const char *appl, enum log_level level,
                     const char *default_outfile)
{
    /* done */
    LOG(level,
        ("usage: %s sfx basic[,<start>[,<end>[,<high>]]]|sys[,<start>]|<jmpaddress> [option]... infile[,<address>]...\n"
         "  The sfx command generates outfiles that are intended to decrunch themselves.\n"
         "  The basic start argument will start a basic program.\n"
         "  The sys start argument will auto detect the start address by searching the\n"
         "  basic start for a sys command.\n"
         "  the <jmpaddress> start argument will jmp to the given address.\n"
         "  -t<target>    sets the decruncher target, must be one of 1, 20, 23, 52, 55\n", appl));
    LOG(level,
        ("                16, 4, 64, 128, 162 or 168, default is 64\n"
         "  -X<custom slow effect assembler fragment>\n"
         "  -x[1-3]|<custom fast effect assembler fragment>\n"
         "                decrunch effect, assembler fragment (don't change X-reg, Y-reg\n"
         "                or carry) or 1 - 3 for different fast border flash effects\n"
         "  -n            no effect, can't be combined with -X or -x\n"
         "  -D<symbol>=<value>\n"
         "                predefines symbols for the sfx assembler\n"));
    LOG(level,
        ("  -s<custom enter assembler fragment>\n"
         "  -f<custom exit assembler fragment>\n"));
    print_crunch_flags(level, default_outfile);
    LOG(level,
        (" All infiles are merged into the outfile. They are loaded in the order\n"
         " they are given on the command-line, from left to right.\n"));
}

static
void print_desfx_usage(const char *appl, enum log_level level,
                       const char *default_outfile)
{
    /* done */
    LOG(level,
        ("usage: %s desfx [option]... infile\n"
         "  The desfx command decrunches files that previously been crunched using the\n"
         "  sfx command.\n", appl));
    LOG(level,
        ("  -e <address>  overrides the automatic entry point detection, using \"load\" as\n"
         "                <address> sets it to the load address of the infile\n"));

    print_base_flags(level, default_outfile);
}

static
void level(const char *appl, int argc, char *argv[])
{
    char flags_arr[32];
    int forward_mode = 0;
    int literal_sequences_used = 0;
    int max_safety = 0;
    int c;
    int infilec;
    char **infilev;

    struct crunch_options options[1] = { CRUNCH_OPTIONS_DEFAULT };
    struct common_flags flags[1] = {{NULL, DEFAULT_OUTFILE}};

    struct membuf in[1];
    struct membuf out[1];

    flags->options = options;

    LOG(LOG_DUMP, ("flagind %d\n", flagind));
    sprintf(flags_arr, "f%s", CRUNCH_FLAGS);
    while ((c = getflag(argc, argv, flags_arr)) != -1)
    {
        LOG(LOG_DUMP, (" flagind %d flagopt '%c'\n", flagind, c));
        switch (c)
        {
        case 'f':
            forward_mode = 1;
            break;
        default:
            handle_crunch_flags(c, flagarg, print_level_usage, appl, flags);
        }
    }

    membuf_init(in);
    membuf_init(out);

    infilev = argv + flagind;
    infilec = argc - flagind;

    if (infilec == 0)
    {
        LOG(LOG_ERROR, ("Error: no input files to process.\n"));
        print_level_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
        exit(-1);
    }

    /* append the files instead of merging them */
    for(c = 0; c < infilec; ++c)
    {
        struct crunch_info info[1];
        int in_load;
        int in_len;
        int out_pos;
        out_pos = membuf_memlen(out);

        in_load = do_load(infilev[c], in);
        in_len = membuf_memlen(in);

        if(forward_mode)
        {
            /* append the starting address of decrunching */
            membuf_append_char(out, in_load >> 8);
            membuf_append_char(out, in_load & 255);

            crunch(in, out, options, info);
        }
        else
        {
            crunch_backwards(in, out, options, info);

            /* append the starting address of decrunching */
            membuf_append_char(out, (in_load + in_len) & 255);
            membuf_append_char(out, (in_load + in_len) >> 8);

            /* reverse the just appended segment of the out buffer */
            reverse_buffer((char*)membuf_get(out) + out_pos,
                           membuf_memlen(out) - out_pos);
        }

        if(info->literal_sequences_used)
        {
            literal_sequences_used = 1;
        }
        if(info->needed_safety_offset > max_safety)
        {
            max_safety = info->needed_safety_offset;
        }
    }

    LOG(LOG_NORMAL, (" Literal sequences are %sused and",
                     literal_sequences_used ? "" : "not "));
    LOG(LOG_NORMAL, (" the largest safety offset is %d.\n",
                     max_safety));

    write_file(flags->outfile, out);

    membuf_free(out);
    membuf_free(in);
}

static
void mem(const char *appl, int argc, char *argv[])
{
    char flags_arr[32];
    int forward_mode = 0;
    int load_addr = -1;
    int prepend_load_addr = 1;
    int c;
    int infilec;
    char **infilev;

    struct crunch_options options[1] = { CRUNCH_OPTIONS_DEFAULT };
    struct common_flags flags[1] = {{NULL, DEFAULT_OUTFILE}};

    struct membuf in[1];
    struct membuf out[1];

    flags->options = options;

    LOG(LOG_DUMP, ("flagind %d\n", flagind));
    sprintf(flags_arr, "fl:%s", CRUNCH_FLAGS);
    while ((c = getflag(argc, argv, flags_arr)) != -1)
    {
        LOG(LOG_DUMP, (" flagind %d flagopt '%c'\n", flagind, c));
        switch(c)
        {
        case 'f':
            forward_mode = 1;
            break;
        case 'l':
            if(strcmp(flagarg, "none") == 0)
            {
                prepend_load_addr = 0;
            }
            else if(str_to_int(flagarg, &load_addr) != 0 ||
                    load_addr < 0 || load_addr >= 65536)
            {
                LOG(LOG_ERROR,
                    ("Error: invalid address for -l option, "
                     "must be in the range of [0 - 0xffff]\n"));
                print_mem_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
                exit(-1);
            }
            break;
        default:
            handle_crunch_flags(c, flagarg, print_mem_usage, appl, flags);
        }
    }

    membuf_init(in);
    membuf_init(out);

    infilev = argv + flagind;
    infilec = argc - flagind;

    if (infilec == 0)
    {
        LOG(LOG_ERROR, ("Error: no input files to process.\n"));
        print_mem_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
        exit(-1);
    }

    {
        struct crunch_info info[1];
        int in_load;
        int in_len;
        int safety;

        in_load = do_loads(infilec, infilev, in, -1, -1, NULL, NULL);
        in_len = membuf_memlen(in);

        LOG(LOG_NORMAL, (" crunching from $%04X to $%04X ",
                         in_load, in_load + in_len));

        /* make room for load addr */
        if(prepend_load_addr)
        {
            membuf_append(out, NULL, 2);
        }

        if(forward_mode)
        {
            /* append the in_loading address of decrunching */
            membuf_append_char(out, in_load >> 8);
            membuf_append_char(out, in_load & 255);

            crunch(in, out, options, info);
            safety = info->needed_safety_offset;
        }
        else
        {
            crunch_backwards(in, out, options, info);
            safety = info->needed_safety_offset;

            /* append the in_loading address of decrunching */
            membuf_append_char(out, (in_load + in_len) & 255);
            membuf_append_char(out, (in_load + in_len) >> 8);
        }

        /* prepend load addr */
        if(prepend_load_addr)
        {
            char *p;
            if(load_addr < 0)
            {
                /* auto load addr specified */
                load_addr = in_load;
                if(forward_mode)
                {
                    load_addr += in_len + safety - membuf_memlen(out) + 2;
                }
                else
                {
                    load_addr -= safety;
                }
            }
            p = membuf_get(out);
            p[0] = load_addr & 255;
            p[1] = load_addr >> 8;
            LOG(LOG_NORMAL, (" The load address is $%04X - $%04X.\n",
                             load_addr, load_addr + membuf_memlen(out) - 2));
        }
        else
        {
            LOG(LOG_NORMAL, (" No load address, data length is $%04X.\n",
                             membuf_memlen(out)));
        }

        LOG(LOG_NORMAL, (" Literal sequences are %sused and",
                         info->literal_sequences_used ? "" : "not "));
        LOG(LOG_NORMAL, (" the safety offset is %d.\n",
                         info->needed_safety_offset));
    }

    write_file(flags->outfile, out);

    membuf_free(out);
    membuf_free(in);
}

static
const struct target_info *
get_target_info(int target)
{
    static const struct target_info targets[] =
        {
            {1,   0xbf, 0x0501, 0x10000, "Oric"},
            {20,  0x9e, 0x1001, 0x2000,  "Vic20"},
            {23,  0x9e, 0x0401, 0x2000,  "Vic20+3kB"},
            {52,  0x9e, 0x1201, 0x8000,  "Vic20+32kB"},
            {55,  0x9e, 0x1201, 0x8000,  "Vic20+3kB+32kB"},
            {16,  0x9e, 0x1001, 0x4000,  "C16"},
            {4,   0x9e, 0x1001, 0xfd00,  "plus4"},
            {64,  0x9e, 0x0801, 0x10000, "C64"},
            {128, 0x9e, 0x1c01, 0xff00,  "C128"},
            {162, 0x8c, 0x0801, 0xc000,  "Apple ][+"},
            {168, -1,   0x2000, 0xd000,  "Atari 400/800 XL/XE"},
            {0,   -1,   -1,     -1,  NULL}
        };
    const struct target_info *targetp;
    for(targetp = targets; targetp->id != 0; ++targetp)
    {
        if(target == targetp->id)
        {
            break;
        }
    }
    if(targetp->id == 0)
    {
        targetp = NULL;
    }
    return targetp;
}

static void do_effect(const char *appl, int no_effect, char *fast, char *slow)
{
    struct membuf *fx = NULL;

    if(no_effect + (fast != NULL) + (slow != NULL) > 1)
    {
        LOG(LOG_ERROR,
            ("Error: can't combine any of the -n, -x or -X flags.\n"));
        print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
        exit(-1);
    }
    if(no_effect)
    {
        set_initial_symbol("i_effect", -1);
    }
    else if(fast != NULL)
    {
        int value;
        if(str_to_int(fast, &value) == 0)
        {
            if(value == 1) set_initial_symbol("i_effect", 1);
            else if(value == 2) set_initial_symbol("i_effect", 2);
            else if(value == 3) set_initial_symbol("i_effect", 3);
            else
            {
                LOG(LOG_ERROR,
                    ("Error: invalid range for effect shorthand, "
                     "must be in the range of [1 - 3]\n"));
                print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
                exit(-1);
            }
        }
        else
        {
            set_initial_symbol("i_effect_custom", 1);
            fx = new_initial_named_buffer("effect_custom");
            membuf_append(fx, fast, strlen(fast));
        }
        set_initial_symbol("i_effect_speed", 1);
    }
    else if(slow != NULL)
    {
        int value;
        if(str_to_int(slow, &value) == 0)
        {
            LOG(LOG_ERROR, ("Error: Can't use shorthand for -X flag.\n"));
            print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
            exit(-1);
        }
        else
        {
            set_initial_symbol("i_effect_custom", 1);
            fx = new_initial_named_buffer("effect_custom");
            membuf_append(fx, slow, strlen(slow));
            set_initial_symbol("i_effect_speed", 0);
        }
    }
    else
    {
        set_initial_symbol("i_effect", 0);
        set_initial_symbol("i_effect_speed", 0);
    }
}

static
void sfx(const char *appl, int argc, char *argv[])
{
    int in_load;
    int in_len;
    int basic_txt_start = -1;
    int basic_var_start = -1;
    int basic_highest_addr = -1;
    int decr_target = 64;
    int sys_addr = -1;
    int no_effect = 0;
    char *fast = NULL;
    char *slow = NULL;
    char *enter_custom = NULL;
    char *exit_custom = NULL;
    char flags_arr[32];
    int c;
    int infilec;
    char **infilev;

    struct crunch_info info[1];

    struct crunch_options options[1] = { CRUNCH_OPTIONS_DEFAULT };
    struct common_flags flags[1] = {{NULL, DEFAULT_OUTFILE}};
    const struct target_info *targetp;

    struct membuf buf1[1];

    struct membuf *in;
    struct membuf *out;

    flags->options = options;

    if(argc <= 1)
    {
        LOG(LOG_ERROR, ("Error: no start argument given.\n"));
        print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
        exit(-1);
    }

    parse_init();

    /* required argument: how to start the crunched program */
    do
    {
        char *p = strtok(argv[1], ",");
        if (strcmp(p, "sys") == 0)
        {
            /* we should look for a basic sys command. */
            sys_addr = -1;
            p = strtok(NULL, ",");
            /* look for an optional basic start address */
            if(p == NULL) break;
            if(str_to_int(p, &basic_txt_start) != 0)
            {
                LOG(LOG_ERROR,
                    ("Error: invalid value for the start of basic text "
                     "address.\n"));
                print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
                exit(-1);
            }
        }
        else if(strcmp(p, "basic") == 0)
        {
            /* we should start a basic program. */
            sys_addr = -2;
            p = strtok(NULL, ",");
            /* look for an optional basic start address */
            if(p == NULL) break;
            if(str_to_int(p, &basic_txt_start) != 0)
            {
                LOG(LOG_ERROR,
                    ("Error: invalid value for the start of basic text "
                     "address.\n"));
                print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
                exit(-1);
            }
            p = strtok(NULL, ",");
            /* look for an optional basic end address */
            if(p == NULL) break;
            if(str_to_int(p, &basic_var_start) != 0)
            {
                LOG(LOG_ERROR,
                    ("Error: invalid value for the start of basic "
                     "variables address.\n"));
                print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
                exit(-1);
            }
            p = strtok(NULL, ",");
            /* look for an optional highest address used by basic */
            if(p == NULL) break;
            if(str_to_int(p, &basic_highest_addr) != 0)
            {
                LOG(LOG_ERROR,
                    ("Error: invalid value for the highest address used "
                     "by basic address.\n"));
                print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
                exit(-1);
            }
        }
        else if(str_to_int(p, &sys_addr) != 0 ||
                sys_addr < 0 || sys_addr >= 65536)
        {
            /* we got an address we should jmp to. */
            LOG(LOG_ERROR,
                ("Error: invalid address for <start>, "
                 "must be in the range of [0 - 0xffff]\n"));
            print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
            exit(-1);
        }
    }
    while(0);

    LOG(LOG_DUMP, ("flagind %d\n", flagind));
    sprintf(flags_arr, "nD:t:x:X:s:f:%s", CRUNCH_FLAGS);
    while ((c = getflag(argc, argv, flags_arr)) != -1)
    {
        char *p;
        LOG(LOG_DUMP, (" flagind %d flagopt '%c'\n", flagind, c));
        switch(c)
        {
        case 't':
            if (str_to_int(flagarg, &decr_target) != 0 ||
                get_target_info(decr_target) == NULL)
            {
                LOG(LOG_ERROR,
                    ("error: invalid value, %d, for -t option, must be one of "
                     "1, 20, 23, 52, 55, 16, 4, 64, 128, 162 or 168.\n",
                     decr_target));
                print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
                exit(-1);
            }
            break;
        case 'n':
            no_effect = 1;
            break;
        case 'x':
            fast = strdup(flagarg);
            break;
        case 'X':
            slow = strdup(flagarg);
            break;
        case 's':
            enter_custom = strdup(flagarg);
            break;
        case 'f':
            exit_custom = strdup(flagarg);
            break;
        case 'D':
            p = strrchr(flagarg, '=');
            if(p != NULL)
            {
                int value;
                if(str_to_int(p + 1, &value) != 0)
                {
                    LOG(LOG_ERROR, ("Error: invalid value for -D "
                                    "<symbol>[=<value>] option.\n"));
                    print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
                    exit(-1);
                }
                /* This is ugly, we really should allocate our own
                 * copy of the symbol string. */
                *p = '\0';
                set_initial_symbol(flagarg, value);
            }
            else
            {
                LOG(LOG_ERROR, ("Error: invalid value for -D "
                                "<symbol>=<value> option.\n"));
                print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
                exit(-1);
            }
            break;
        default:
            handle_crunch_flags(c, flagarg, print_sfx_usage, appl, flags);
        }
    }

    do_effect(appl, no_effect, fast, slow);
    if(enter_custom != NULL)
    {
        set_initial_symbol("i_enter_custom", 1);
        membuf_append(new_initial_named_buffer("enter_custom"),
                      enter_custom, strlen(enter_custom));
    }
    if(exit_custom != NULL)
    {
        set_initial_symbol("i_exit_custom", 1);
        membuf_append(new_initial_named_buffer("exit_custom"),
                      exit_custom, strlen(exit_custom));
    }

    membuf_init(buf1);
    in = buf1;
    out = new_initial_named_buffer("crunched_data");

    infilev = argv + flagind + 1;
    infilec = argc - flagind - 1;

    if (infilec == 0)
    {
        LOG(LOG_ERROR, ("Error: no input files to process.\n"));
        print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
        exit(-1);
    }

    targetp = get_target_info(decr_target);
    if(sys_addr == -2 && targetp->id == 168)
    {
        /* basic start not implemented for Apple and Atari targets */
        LOG(LOG_ERROR, ("Start address \"basic\" is not supported for "
                        "the %s target.\n", targetp->model));
        print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
        exit(-1);
    }
    if(sys_addr == -1 && targetp->id == 162)
    {
        /* auto detecting of start address not implemented for Apple target */
        LOG(LOG_ERROR, ("Start address \"sys\" is not supported for "
                        "the %s target\n", targetp->model));
        print_sfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
        exit(-1);
    }

    if(basic_txt_start == -1)
    {
        basic_txt_start = targetp->basic_txt_start;
    }

    {
        int safety;
        int *basic_var_startp;
        int *sys_addrp;
        int basic_start;

        sys_addrp = NULL;
        basic_var_startp = NULL;
        if(sys_addr == -2 && basic_var_start == -1)
        {
            basic_var_startp = &basic_var_start;
        }
        basic_start = -1;
        if(sys_addr < 0)
        {
            basic_start = basic_txt_start;
            if(sys_addr == -1)
            {
                sys_addrp = &sys_addr;
            }
        }

        in_load = do_loads(infilec, infilev, in,
                           basic_start, targetp->sys_token,
                           basic_var_startp, sys_addrp);
        in_len = membuf_memlen(in);

        if(in_load + in_len > targetp->end_of_ram)
        {
            LOG(LOG_ERROR, ("Error:\n The memory of the %s target ends at "
                            "$%04X and can't hold the uncrunched data\n "
                            "that covers $%04X to $%04X.\n",
                            targetp->model, targetp->end_of_ram,
                            in_load, in_load + in_len));
            exit(-1);
        }

        LOG(LOG_NORMAL, (" crunching from $%04X to $%04X ",
                         in_load, in_load + in_len));

        if(decr_target == 20 || decr_target == 52)
        {
            /* these are vic20 targets with a memory hole from
             * $0400-$1000. Each page is filled with the value of the
             * high-byte of its address. */
            if(in_load >= 0x0400 && in_load + in_len <= 0x1000)
            {
                /* all the loaded data is in the memory hole.*/
                LOG(LOG_ERROR,
                    ("Error: all data loaded to the memory hole.\n"));
                exit(-1);
            }
            else if(in_load >= 0x0400 && in_load < 0x1000 &&
                    in_load + in_len > 0x1000)
            {
                /* The data starts in the memory hole and ends in
                 * RAM. We need to adjust the start. */
                int diff = 0x1000 - in_load;
                in_load += diff;
                in_len -= diff;
                membuf_trim(in, diff);
                LOG(LOG_WARNING,
                    ("Warning, trimming address interval to $%04X-$%04X.\n",
                     in_load, in_load + in_len));
            }
            else if(in_load < 0x0400 &&
                    in_load + in_len >= 0x0400 && in_load + in_len < 0x1000)
            {
                /* The data starts in RAM and ends in the memory
                 * hole. We need to adjust the end. */
                int diff = in_load + in_len - 0x0400;
                in_len -= diff;
                membuf_truncate(in, in_len);
                LOG(LOG_WARNING,
                    ("Warning, trimming address interval to $%04X-$%04X.\n",
                     in_load, in_load + in_len));
            }
            else if(in_load < 0x0400 && in_load + in_len > 0x1000)
            {
                /* The data starts in RAM covers the memory hole and
                 * ends in RAM. */
                int hi, lo;
                char *p;
                p = membuf_get(in);
                for(hi = 0x04; hi < 0x10; hi += 0x01)
                {
                    for(lo = 0; lo < 256; ++lo)
                    {
                        int addr = (hi << 8) | lo;
                        p[addr - in_load] = hi;
                    }
                }
                LOG(LOG_NORMAL, ("Memory hole at interval $0400-$1000 "
                                 "included in crunch..\n"));
            }
        }

        /* make room for load addr */
        membuf_append(out, NULL, 2);

        crunch_backwards(in, out, options, info);
        safety = info->needed_safety_offset;

        /* append the in_loading address of decrunching */
        membuf_append_char(out, (in_load + in_len) & 255);
        membuf_append_char(out, (in_load + in_len) >> 8);

        /* prepend load addr */
        {
            char *p;
            p = membuf_get(out);
            p[0] = (in_load - safety) & 255;
            p[1] = (in_load - safety) >> 8;
        }
    }

    LOG(LOG_NORMAL, (" Target is self-decrunching %s executable",
                     targetp->model));
    if(sys_addr == -1)
    {
        LOG(LOG_ERROR, ("\nError: can't find sys address (token $%02X)"
                        " at basic text start ($%04X).\n",
                        targetp->sys_token, basic_txt_start));
        exit(-1);
    }
    if(sys_addr != -2)
    {
        LOG(LOG_NORMAL, (",\n jmp address $%04X.\n", sys_addr));
    }
    else
    {
        LOG(LOG_NORMAL, (",\n basic start ($%04X-$%04X).\n",
                         basic_txt_start, basic_var_start));
    }

    {
        /* add decruncher */
        struct membuf source[1];

        membuf_init(source);
        decrunch(LOG_DEBUG, sfxdecr, source);

        in = out;
        out = buf1;
        membuf_clear(out);

        set_initial_symbol("r_start_addr", sys_addr);
        /*initial_symbol_dump( LOG_NORMAL, "r_start_addr");*/
        set_initial_symbol("r_target", decr_target);
        /*initial_symbol_dump( LOG_NORMAL, "r_target");*/
        set_initial_symbol("r_in_load", in_load);
        /*initial_symbol_dump( LOG_NORMAL, "r_in_addr");*/
        set_initial_symbol("r_in_len", in_len);
        /*initial_symbol_dump( LOG_NORMAL, "r_in_len");*/

        if(sys_addr == -2)
        {
            /* only set this if its changed from the default. */
            if(basic_txt_start != targetp->basic_txt_start)
            {
                set_initial_symbol("i_basic_txt_start", basic_txt_start);
                initial_symbol_dump( LOG_DEBUG, "i_basic_txt_start");
            }
            /* only set this if we've been given a value for it. */
            if(basic_var_start != -1)
            {
                set_initial_symbol("i_basic_var_start", basic_var_start);
                initial_symbol_dump(LOG_DEBUG, "i_basic_var_start");
            }
            /* only set this if we've been given a value for it. */
            if(basic_highest_addr != -1)
            {
                set_initial_symbol("i_basic_highest_addr", basic_highest_addr);
                initial_symbol_dump(LOG_DEBUG, "i_basic_highest_addr");
            }
        }

        if(info->literal_sequences_used)
        {
            set_initial_symbol("i_literal_sequences_used", 1);
            initial_symbol_dump(LOG_DEBUG, "i_literal_sequences_used");
        }

        if(assemble(source, out) != 0)
        {
            LOG(LOG_ERROR, ("Parse failure.\n"));
        }
        else
        {
            i32 v_safety_addr;
            i32 transfer_len;
            i32 i_table_addr;
            i32 i_effect;
            i32 c_effect_color;
            i32 cover_start = in_load;

            resolve_symbol("v_safety_addr", NULL, &v_safety_addr);
            resolve_symbol("transfer_len", NULL, &transfer_len);
            resolve_symbol("i_table_addr", NULL, &i_table_addr);
            resolve_symbol("i_effect2", NULL, &i_effect);

            if(transfer_len != 0)
            {
                cover_start = v_safety_addr;
            }

            LOG(LOG_NORMAL, ("Memory layout:\n"));
            LOG(LOG_NORMAL, (" Data covers $%04X to $%04X.\n",
                             cover_start, in_load + in_len));
            LOG(LOG_NORMAL, (" Decrunch table is located at $%04X to $%04X.\n",
                             i_table_addr, i_table_addr + 156));
            if(i_effect == 0 && !resolve_symbol("i_effect_custom", NULL, NULL))
            {
                resolve_symbol("c_effect_color", NULL, &c_effect_color);
                LOG(LOG_NORMAL, (" Decrunch effect writes to $%04X.\n",
                                 c_effect_color));
            }
        }

        membuf_free(source);
    }

    write_file(flags->outfile, out);

    membuf_free(buf1);

    parse_free();
}

static
void raw(const char *appl, int argc, char *argv[])
{
    char flags_arr[32];
    int decrunch_mode = 0;
    int backwards_mode = 0;
    int reverse_mode = 0;
    int c, infilec;
    char **infilev;

    static struct crunch_options options[1] = { CRUNCH_OPTIONS_DEFAULT };
    struct common_flags flags[1] = { {options, DEFAULT_OUTFILE} };

    struct membuf inbuf[1];
    struct membuf outbuf[1];

    LOG(LOG_DUMP, ("flagind %d\n", flagind));
    sprintf(flags_arr, "bdr%s", CRUNCH_FLAGS);
    while ((c = getflag(argc, argv, flags_arr)) != -1)
    {
        LOG(LOG_DUMP, (" flagind %d flagopt '%c'\n", flagind, c));
        switch (c)
        {
        case 'b':
            backwards_mode = 1;
            break;
        case 'r':
            reverse_mode = 1;
            break;
        case 'd':
            decrunch_mode = 1;
            break;
        default:
            handle_crunch_flags(c, flagarg, print_raw_usage, appl, flags);
        }
    }

    infilev = argv + flagind;
    infilec = argc - flagind;

    if (infilec != 1)
    {
        LOG(LOG_ERROR, ("Error: exactly one input file must be given.\n"));
        print_raw_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
        exit(-1);
    }

    membuf_init(inbuf);
    membuf_init(outbuf);

    load_plain_file(infilev[0], inbuf);

    if(decrunch_mode)
    {
        int seems_backward = 0;
        int seems_forward = 0;
        unsigned char *p;

        p = membuf_get(inbuf);
        if(p[0] == 0x80 && p[1] == 0x0)
        {
            seems_backward = 1;
        }
        p += membuf_memlen(inbuf);
        if(p[-1] == 0x80 && p[-2] == 0x0)
        {
            seems_forward = 1;
        }

        /* do we know what way it was crunched? */
        if((seems_backward ^ seems_forward) != 0)
        {
            /* yes, override option. */
            backwards_mode = seems_backward;
        }

        if(backwards_mode)
        {
            LOG(LOG_NORMAL, ("Decrunching infile \"%s\" to outfile \"%s\" "
                             " backwards.\n", infilev[0], flags->outfile));
            decrunch_backwards(LOG_NORMAL, inbuf, outbuf);
        }
        else
        {
            LOG(LOG_NORMAL, ("Decrunching infile \"%s\" to outfile \"%s\".\n",
                             infilev[0], flags->outfile));
            decrunch(LOG_NORMAL, inbuf, outbuf);
        }
    }
    else
    {
        struct crunch_info info[1];
        if(backwards_mode)
        {
            LOG(LOG_NORMAL, ("Crunching infile \"%s\" to outfile \"%s\" "
                             "backwards.\n", infilev[0], flags->outfile));
            crunch_backwards(inbuf, outbuf, options, info);
        }
        else
        {
            LOG(LOG_NORMAL, ("Crunching infile \"%s\" to outfile \"%s\".\n",
                             infilev[0], flags->outfile));
            crunch(inbuf, outbuf, options, info);
        }

        LOG(LOG_NORMAL, (" Literal sequences are %sused and",
                         info->literal_sequences_used ? "" : "not "));
        LOG(LOG_NORMAL, (" the safety offset is %d.\n",
                         info->needed_safety_offset));

    }

    if(reverse_mode)
    {
        reverse_buffer(membuf_get(outbuf), membuf_memlen(outbuf));
    }

    write_file(flags->outfile, outbuf);

    membuf_free(outbuf);
    membuf_free(inbuf);
}

static
void desfx(const char *appl, int argc, char *argv[])
{
    char flags_arr[32];
    struct load_info info[1];
    struct membuf mem[1];
    const char *outfile = DEFAULT_OUTFILE;
    int c, infilec;
    char **infilev;
    u8 *p;
    u16 start;
    u16 end;
    int cookedend;
    int entry = -1;

    LOG(LOG_DUMP, ("flagind %d\n", flagind));
    sprintf(flags_arr, "e:%s", BASE_FLAGS);
    while ((c = getflag(argc, argv, flags_arr)) != -1)
    {
        LOG(LOG_DUMP, (" flagind %d flagopt '%c'\n", flagind, c));
        switch (c)
        {
        case 'e':
            if(strcmp(flagarg,"load") == 0)
            {
                entry = -2;
            }
            else if(str_to_int(flagarg, &entry) != 0 ||
                    entry < 0 || entry >= 65536)
            {
                LOG(LOG_ERROR,("Error: invalid address for -e option, "
                               "must be in the range of [0 - 0xffff]\n"));
                print_desfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
                exit(-1);
            }
            break;
        default:
            handle_base_flags(c, flagarg, print_desfx_usage, appl, &outfile);
        }
    }

    infilev = argv + flagind;
    infilec = argc - flagind;

    if (infilec != 1)
    {
        LOG(LOG_ERROR, ("Error: exactly one input file must be given.\n"));
        print_desfx_usage(appl, LOG_NORMAL, DEFAULT_OUTFILE);
        exit(-1);
    }

    membuf_init(mem);
    membuf_append(mem, NULL, 65536);

    p = membuf_get(mem);

    /* load file, don't care about tracking basic*/
    info->basic_txt_start = -1;
    load_located(infilev[0], p, info);

    if(entry == -1)
    {
        /* use detected address */
        entry = info->run;
    }
    else if(entry == -2)
    {
        /* use load address */
        entry = info->start;
    }

    /* no start address from load */
    if(entry == -1)
    {
        /* look for sys line */
        entry = find_sys(p + info->start, -1);
    }
    if(entry == -1)
    {
        LOG(LOG_ERROR, ("Error, can't find entry point.\n"));
        exit(-1);
    }

    LOG(LOG_NORMAL, (" crunched file entry point $%04X\n", entry));
    entry = decrunch_sfx(p, entry, &start, &end);

    /* change 0x0 into 0x10000 */
    cookedend = ((end - 1) & 0xffff) + 1;

    LOG(LOG_NORMAL, (" decrunched entry point $%04X, from $%04X to $%04X\n",
                     entry, start, cookedend));

    membuf_truncate(mem, cookedend);
    membuf_trim(mem, start);
    membuf_insert(mem, 0, NULL, 2);

    p = membuf_get(mem);
    p[0] = start;
    p[1] = start >> 8;

    write_file(outfile, mem);

    membuf_free(mem);
}

int
main(int argc, char *argv[])
{
    const char *appl;

    /* init logging */
    LOG_INIT_CONSOLE(LOG_NORMAL);

    appl = fixup_appl(argv[0]);
    if(argc < 2)
    {
        /* missing required command */
        LOG(LOG_ERROR, ("Error: required command is missing.\n"));
        print_command_usage(appl, LOG_ERROR);
        exit(-1);
    }
    ++argv;
    --argc;
    if(strcmp(argv[0], "level") == 0)
    {
        level(appl, argc, argv);
    }
    else if(strcmp(argv[0], "mem") == 0)
    {
        mem(appl, argc, argv);
    }
    else if(strcmp(argv[0], "sfx") == 0)
    {
        sfx(appl, argc, argv);
    }
    else if(strcmp(argv[0], "raw") == 0)
    {
        raw(appl, argc, argv);
    }
    else if(strcmp(argv[0], "desfx") == 0)
    {
        desfx(appl, argc, argv);
    }
    else if(strcmp(argv[0], "-v") == 0)
    {
        print_license();
    }
    else if(strcmp(argv[0], "-?") == 0)
    {
        print_command_usage(appl, LOG_NORMAL);
    }
    else
    {
        /* unknown command */
        LOG(LOG_ERROR,
            ("Error: unrecognised command \"%s\".\n", argv[0]));
        print_command_usage(appl, LOG_ERROR);
        exit(-1);
    }

    LOG_FREE;

    return 0;
}
