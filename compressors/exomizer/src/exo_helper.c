/*
 * Copyright (c) 2005, 2013, 2015 Magnus Lind.
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

#include "log.h"
#include "output.h"
#include "membuf.h"
#include "match.h"
#include "search.h"
#include "optimal.h"
#include "exodec.h"
#include "exo_helper.h"
#include "exo_util.h"
#include "getflag.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static struct crunch_options default_options[1] = { CRUNCH_OPTIONS_DEFAULT };

int do_output(match_ctx ctx,
              struct search_node *snp,
              encode_match_data emd,
              encode_match_f * f,
              struct membuf *outbuf,
              int *literal_sequences_used,
              int output_header)
{
    int pos;
    int pos_diff;
    int max_diff;
    int diff;
    int copy_used = 0;
    output_ctxp old;
    output_ctx out;

    output_ctx_init(out, outbuf);
    old = emd->out;
    emd->out = out;

    pos = output_get_pos(out);

    pos_diff = pos;
    max_diff = 0;

    LOG(LOG_DUMP, ("pos $%04X\n", out->pos));
    output_gamma_code(out, 16);
    output_bits(out, 1, 0); /* 1 bit out */

    diff = output_get_pos(out) - pos_diff;
    if(diff > max_diff)
    {
        max_diff = diff;
    }

    LOG(LOG_DUMP, ("pos $%04X\n", out->pos));
    LOG(LOG_DUMP, ("------------\n"));

    while (snp != NULL)
    {
        const_matchp mp;

        mp = snp->match;
        if (mp != NULL && mp->len > 0)
        {
            if (mp->offset == 0)
            {
                if(mp->len == 1)
                {
                    /* literal */
                    LOG(LOG_DUMP, ("literal byte: $%02X\n",
                                   ctx->buf[snp->index]));
                    output_byte(out, ctx->buf[snp->index]);
                    output_bits(out, 1, 1);
                } else
                {
                    int i;
                    for(i = 0; i < mp->len; ++i)
                    {
                        output_byte(out, ctx->buf[snp->index + i]);
                    }
                    output_bits(out, 16, mp->len);
                    output_gamma_code(out, 17);
                    output_bits(out, 1, 0);
                    copy_used = 1;
                }
            } else
            {
                f(mp, emd, NULL);
                output_bits(out, 1, 0);
            }

            pos_diff += mp->len;
            diff = output_get_pos(out) - pos_diff;
            if(diff > max_diff)
            {
                max_diff = diff;
            }
        }
        LOG(LOG_DUMP, ("------------\n"));
        snp = snp->prev;
    }

    LOG(LOG_DUMP, ("pos $%04X\n", out->pos));
    if (output_header)
    {
        /* output header here */
        optimal_out(out, emd);
        LOG(LOG_DUMP, ("pos $%04X\n", out->pos));
    }
    output_bits_flush(out);

    emd->out = old;

    if(literal_sequences_used != NULL)
    {
        *literal_sequences_used = copy_used;
    }

    return max_diff;
}

struct search_node*
do_compress(match_ctx ctx, encode_match_data emd,
            const char *exported_encoding,
            int max_passes,
            int use_literal_sequences,
            struct membuf *enc)
{
    matchp_cache_enum mpce;
    matchp_snp_enum snpe;
    struct search_node *snp;
    struct search_node *best_snp;
    int pass;
    float size;
    float old_size;
    char prev_enc[100];

    pass = 1;
    prev_enc[0] = '\0';

    LOG(LOG_NORMAL, (" pass %d: ", pass));
    if(exported_encoding != NULL)
    {
        LOG(LOG_NORMAL, ("importing %s\n", exported_encoding));
        optimal_encoding_import(emd, exported_encoding);
    }
    else
    {
        LOG(LOG_NORMAL, ("optimizing ..\n"));
        matchp_cache_get_enum(ctx, mpce);
        optimal_optimize(emd, matchp_cache_enum_get_next, mpce);
    }
    optimal_encoding_export(emd, enc);
    strcpy(prev_enc, membuf_get(enc));

    best_snp = NULL;
    old_size = 100000000.0;

    for (;;)
    {
        snp = search_buffer(ctx, optimal_encode, emd,
                            use_literal_sequences);
        if (snp == NULL)
        {
            LOG(LOG_ERROR, ("error: search_buffer() returned NULL\n"));
            exit(1);
        }

        size = snp->total_score;
        LOG(LOG_NORMAL, ("  size %0.1f bits ~%d bytes\n",
                         size, (((int) size) + 7) >> 3));

        if (size >= old_size)
        {
            free(snp);
            break;
        }

        if (best_snp != NULL)
        {
            free(best_snp);
        }
        best_snp = snp;
        old_size = size;
        ++pass;

        if(pass > max_passes)
        {
            break;
        }

        optimal_free(emd);
        optimal_init(emd);

        LOG(LOG_NORMAL, (" pass %d: optimizing ..\n", pass));

        matchp_snp_get_enum(snp, snpe);
        optimal_optimize(emd, matchp_snp_enum_get_next, snpe);

        optimal_encoding_export(emd, enc);
        if (strcmp(membuf_get(enc), prev_enc) == 0)
        {
            break;
        }
        strcpy(prev_enc, membuf_get(enc));
    }

    return best_snp;
}

void crunch_backwards(struct membuf *inbuf,
                      struct membuf *outbuf,
                      struct crunch_options *options, /* IN */
                      struct crunch_info *info) /* OUT */
{
    match_ctx ctx;
    encode_match_data emd;
    struct search_node *snp;
    int outlen;
    int inlen;
    int safety;
    int copy_used;
    struct membuf exported_enc = STATIC_MEMBUF_INIT;

    if(options == NULL)
    {
        options = default_options;
    }

    inlen = membuf_memlen(inbuf);
    outlen = membuf_memlen(outbuf);
    emd->out = NULL;
    optimal_init(emd);

    LOG(LOG_NORMAL,
        ("\nPhase 1: Instrumenting file"
         "\n-----------------------------\n"));
    LOG(LOG_NORMAL, (" Length of indata: %d bytes.\n", inlen));

    match_ctx_init(ctx, inbuf, options->max_len, options->max_offset,
                   options->favor_speed);

    LOG(LOG_NORMAL, (" Instrumenting file, done.\n"));

    emd->out = NULL;
    optimal_init(emd);

    LOG(LOG_NORMAL,
        ("\nPhase 2: Calculating encoding"
         "\n-----------------------------\n"));
    snp = do_compress(ctx, emd, options->exported_encoding,
                      options->max_passes, options->use_literal_sequences,
                      &exported_enc);
    LOG(LOG_NORMAL, (" Calculating encoding, done.\n"));

    LOG(LOG_NORMAL,
        ("\nPhase 3: Generating output file"
         "\n------------------------------\n"));
    LOG(LOG_NORMAL, (" Encoding: %s\n", (char*)membuf_get(&exported_enc)));
    safety = do_output(ctx, snp, emd, optimal_encode, outbuf,
                       &copy_used, options->output_header);
    outlen = membuf_memlen(outbuf) - outlen;
    LOG(LOG_NORMAL, (" Length of crunched data: %d bytes.\n", outlen));

    LOG(LOG_BRIEF, (" Crunched data reduced %d bytes (%0.2f%%)\n",
                    inlen - outlen, 100.0 * (inlen - outlen) / inlen));

    optimal_free(emd);
    free(snp);
    match_ctx_free(ctx);

    if(info != NULL)
    {
        info->literal_sequences_used = copy_used;
        info->needed_safety_offset = safety;
        strncpy(info->used_encoding, (char*)membuf_get(&exported_enc), 100);
        info->used_encoding[99] = '\0';
    }
    membuf_free(&exported_enc);
}

void reverse_buffer(char *start, int len)
{
    char *end = start + len - 1;
    char tmp;

    while (start < end)
    {
        tmp = *start;
        *start = *end;
        *end = tmp;

        ++start;
        --end;
    }
}

void crunch(struct membuf *inbuf,
            struct membuf *outbuf,
            struct crunch_options *options, /* IN */
            struct crunch_info *info) /* OUT */
{
    int outpos;
    reverse_buffer(membuf_get(inbuf), membuf_memlen(inbuf));
    outpos = membuf_memlen(outbuf);

    crunch_backwards(inbuf, outbuf, options, info);

    reverse_buffer(membuf_get(inbuf), membuf_memlen(inbuf));
    reverse_buffer((char*)membuf_get(outbuf) + outpos,
                   membuf_memlen(outbuf) - outpos);
}

void decrunch(int level,
              struct membuf *inbuf,
              struct membuf *outbuf)
{
    struct dec_ctx ctx[1];
    struct membuf enc_buf[1] = {STATIC_MEMBUF_INIT};
    dec_ctx_init(ctx, inbuf, outbuf, enc_buf);

    LOG(level, (" Encoding: %s\n", (char*)membuf_get(enc_buf)));

    membuf_free(enc_buf);
    dec_ctx_decrunch(ctx);
    dec_ctx_free(ctx);
}

void decrunch_backwards(int level,
                        struct membuf *inbuf,
                        struct membuf *outbuf)
{
    int outpos;
    reverse_buffer(membuf_get(inbuf), membuf_memlen(inbuf));
    outpos = membuf_memlen(outbuf);

    decrunch(level, inbuf, outbuf);

    reverse_buffer(membuf_get(inbuf), membuf_memlen(inbuf));
    reverse_buffer((char*)membuf_get(outbuf) + outpos,
                   membuf_memlen(outbuf) - outpos);
}

void print_license(void)
{
    LOG(LOG_WARNING,
        ("----------------------------------------------------------------------------\n"
         "Exomizer v2.0.10 Copyright (c) 2002-2017 Magnus Lind. (magli143@gmail.com)\n"
         "----------------------------------------------------------------------------\n"));
    LOG(LOG_WARNING,
        ("This software is provided 'as-is', without any express or implied warranty.\n"
         "In no event will the authors be held liable for any damages arising from\n"
         "the use of this software.\n"
         "Permission is granted to anyone to use this software, alter it and re-\n"
         "distribute it freely for any non-commercial, non-profit purpose subject to\n"
         "the following restrictions:\n\n"));
    LOG(LOG_WARNING,
        ("   1. The origin of this software must not be misrepresented; you must not\n"
         "   claim that you wrote the original software. If you use this software in a\n"
         "   product, an acknowledgment in the product documentation would be\n"
         "   appreciated but is not required.\n"
         "   2. Altered source versions must be plainly marked as such, and must not\n"
         "   be misrepresented as being the original software.\n"
         "   3. This notice may not be removed or altered from any distribution.\n"));
    LOG(LOG_WARNING,
        ("   4. The names of this software and/or it's copyright holders may not be\n"
         "   used to endorse or promote products derived from this software without\n"
         "   specific prior written permission.\n"
         "----------------------------------------------------------------------------\n"
         "The files processed and/or generated by using this software are not covered\n"
         "nor affected by this license in any way.\n"));
}

void print_base_flags(enum log_level level, const char *default_outfile)
{
    LOG(level,
        ("  -o <outfile>  sets the outfile name, default is \"%s\"\n",
         default_outfile));
    LOG(level,
        ("  -q            quiet mode, disables all display output\n"
         "  -B            brief mode, disables most display output\n"
         "  -v            displays version and the usage license\n"
         "  --            treats all following arguments as non-options\n"
         "  -?            displays this help screen\n"));
}

void print_crunch_flags(enum log_level level, const char *default_outfile)
{
    LOG(level,
        ("  -c            compatibility mode, disables the use of literal sequences\n"
         "  -C            favor compression speed over ratio\n"
         "  -e <encoding> uses the given encoding for crunching\n"
         "  -E            don't write the encoding to the outfile\n"
         "  -m <offset>   sets the maximum sequence offset, default is 65535\n"
         "  -M <length>   sets the maximum sequence length, default is 65535\n"
         "  -p <passes>   limits the number of optimization passes, default is 65535\n"));
    print_base_flags(level, default_outfile);
}

void handle_base_flags(int flag_char, /* IN */
                       const char *flag_arg, /* IN */
                       print_usage_f *print_usage, /* IN */
                       const char *appl, /* IN */
                       const char **default_outfilep) /* IN */
{
    switch(flag_char)
    {
    case 'o':
        *default_outfilep = flag_arg;
        break;
    case 'q':
        LOG_SET_LEVEL(LOG_WARNING);
        break;
    case 'B':
        LOG_SET_LEVEL(LOG_BRIEF);
        break;
    case 'v':
        print_license();
        exit(0);
    default:
        if (flagflag != '?')
        {
            LOG(LOG_ERROR,
                ("error, invalid option \"-%c\"", flagflag));
            if (flagarg != NULL)
            {
                LOG(LOG_ERROR, (" with argument \"%s\"", flagarg));
            }
            LOG(LOG_ERROR, ("\n"));
        }
        print_usage(appl, LOG_WARNING, *default_outfilep);
        exit(0);
    }
}

void handle_crunch_flags(int flag_char, /* IN */
                         const char *flag_arg, /* IN */
                         print_usage_f *print_usage, /* IN */
                         const char *appl, /* IN */
                         struct common_flags *flags) /* OUT */
{
    struct crunch_options *options = flags->options;
    switch(flag_char)
    {
    case 'c':
        options->use_literal_sequences = 0;
        break;
    case 'C':
        options->favor_speed = 1;
        break;
    case 'e':
        options->exported_encoding = flag_arg;
        break;
    case 'E':
        options->output_header = 0;
        break;
    case 'm':
        if (str_to_int(flag_arg, &options->max_offset) != 0 ||
            options->max_offset < 0 || options->max_offset >= 65536)
        {
            LOG(LOG_ERROR,
                ("Error: invalid offset for -m option, "
                 "must be in the range of [0 - 65535]\n"));
            print_usage(appl, LOG_NORMAL, flags->outfile);
            exit(1);
        }
        break;
    case 'M':
        if (str_to_int(flag_arg, &options->max_len) != 0 ||
            options->max_len < 0 || options->max_len >= 65536)
        {
            LOG(LOG_ERROR,
                ("Error: invalid offset for -n option, "
                 "must be in the range of [0 - 65535]\n"));
            print_usage(appl, LOG_NORMAL, flags->outfile);
            exit(1);
        }
        break;
    case 'p':
        if (str_to_int(flag_arg, &options->max_passes) != 0 ||
            options->max_passes < 1 || options->max_passes >= 65536)
        {
            LOG(LOG_ERROR,
                ("Error: invalid value for -p option, "
                 "must be in the range of [1 - 65535]\n"));
            print_usage(appl, LOG_NORMAL, flags->outfile);
            exit(1);
        }
        break;
    default:
        handle_base_flags(flag_char, flag_arg, print_usage,
                          appl, &flags->outfile);
    }
}
