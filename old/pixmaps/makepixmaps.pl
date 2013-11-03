#!/usr/local/bin/perl -w


my @files = glob("*.xpm");


open(F, "> ../src/pixmaps.c");
open(F2, "> ../src/pixmaps.h");

print F2 qq/

#ifndef __PIXMAPS_H__
#define __PIXMAPS_H__

#ifdef __cplusplus
extern "C"
{
#endif

/;

print F "#include \"pixmaps.h\"\n";

my $filename;

foreach $filename (@files)
{
   my $basename = substr($filename, 0, -4); ## assume filename ends in .xpm
   system("put_in_name.pl $filename");

   print F "#include \"../pixmaps/$filename\"\n";
   print F2 "extern const char *${basename}_xpm\[\];\n";
}

print F2 qq/

#ifdef __cplusplus
}
#endif

#endif

/;


close F;
close F2;
