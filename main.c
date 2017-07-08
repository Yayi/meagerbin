/*
 * meagerbin - Merge multiple bin files for one
 *
 * Copyright (C) 2017, Yayi<yz@yayi.biz>
 *
 * meagerbin is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * meagerbin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with shadowsocks-libev; see the file COPYING. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define PROGRAM_VERSION "0.0.1"

struct configure{
    int source_list;
    char *source_path[256];
    long int file_offset[256];
    char *null_fill;
}config;

void help()
{
    printf("\n");
    printf("meragebin :%s \n\n",PROGRAM_VERSION);
    printf("maintained by Yayi<yz@yayi.biz>\n\n");
    printf("\tusage:\n");
    printf("\t\tmeragebin -o 0x0 -s 1.bin -o 0x20000 -s 2.bin -f 0 \n\n");
    printf("\t[-h]\t\tPrint this message.\n");
    printf("\t[-o]\t\tAddress offset.\n");
    printf("\t[-s]\t\tSource bin file.\n");
    printf("\t[-f]\t\tFill the extra.\n");
    printf("\n");

}

int captolow(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + 'a' - 'A';
    }
    else
    {
        return c;
    }
}

long int hextoint(char x[])
{
    int i;
    long int y = 0;

    if (x[0] == '0' && (x[1]=='x' || x[1]=='X'))
    {
        i = 2;
    }
    else
    {
        i = 0;
    }
    for (; (x[i] >= '0' && x[i] <= '9') || (x[i] >= 'a' && x[i] <= 'z') || (x[i] >='A' && x[i] <= 'Z');++i)
    {
        if (captolow(x[i]) > '9')
        {
            y = 16 * y + (10 + captolow(x[i]) - 'a');
        }
        else
        {
            y = 16 * y + (captolow(x[i]) - '0');
        }
    }
    return y;
}

int process( )
{
    FILE *out_file;
    int i;
    char fill_ff[1]={0xff};
    char fill_zero[1]={0x00};

    if(config.source_list<=0)
        return -1;

    out_file=fopen("out.bin","w");
    if(out_file==NULL)
    {
        printf("Can not creat file\n");
        return -1;
    }

    for(i=0;i<config.source_list;i++)
    {
        FILE *file;
        file=fopen(config.source_path[i],"r");
        if(file==NULL)
        {
            printf("not find the file: %s\n",config.source_path[i]);
            fclose(out_file);
            return -1;
        }
        else
        {
            int len;
            char *data;
            fseek(file,0L,SEEK_END);
            len=ftell(file);
            fseek(file,0L,SEEK_SET);
            data=(char *)malloc(len+1);
            fread(data,1,len,file);
            if(config.file_offset[i]==0)
                fwrite(data,1,len-1,out_file);
            else
            {
                long int y=0;
                long int x=config.file_offset[i]-ftell(out_file);
                printf("%d",config.file_offset[i]);
                if(config.null_fill[0]=='0')
                {
                    for(y=0;y<x;y++)
                    {
                        fwrite(fill_zero,1,1,out_file);
                    }
                }
                else if(config.null_fill[0]=='f')
                {
                    for(y=0;y<x;y++)
                    {
                        fwrite(fill_ff,1,1,out_file);
                    }
                }
                fwrite(data,1,len-1,out_file);
            }
        }

        fclose(file);
    }
    fclose(out_file);
    return 0;
}


int main(int argc,char **argv)
{
    int c;
    int longIndex = 0;
    config.source_list=0;
    config.null_fill=NULL;
    opterr=1;

    if (argc<2)
    {
        printf("meragebin:Miss any parameters.\nTry \'meragebin --help\' for more information.\n");
        return 1;
    }

    static struct option long_options[] = {
            { "fill",   no_argument,    NULL, 'f' },
            { "offset", no_argument,    NULL, 'o' },
            { "help",   no_argument,    NULL, 'h' },
            { "source", no_argument,    NULL, 's' },
    };

    while ((c = getopt_long(argc, argv, "o:f:s:h?", long_options, &longIndex)) != -1)
    {
        opterr=0;
        switch (c)
        {
            case 'f':
                config.null_fill=optarg;
                if(config.null_fill[0]!='f'&&config.null_fill[0]!='0')
                {
                    help();
                    exit(EXIT_FAILURE);
                }
                break;

            case 'h':
                help();
                exit(EXIT_SUCCESS);

            case 's':
                config.source_path[config.source_list]=optarg;
                config.source_list++;
                break;

            case 'o':
                config.file_offset[config.source_list]=hextoint(optarg);
                break;

            case '?':
                help();
                exit(EXIT_FAILURE);

            default:
                opterr=1;
                break;
        }

    }

    if(config.source_list<=0||config.null_fill==NULL)
    {
        help();
        exit(EXIT_FAILURE);
    }

    if (opterr)
    {
        help();
        exit(EXIT_FAILURE);
    }


    process();

    return EXIT_SUCCESS;
}