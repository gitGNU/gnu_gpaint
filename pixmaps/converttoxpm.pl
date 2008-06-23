#!/usr/local/bin/perl -w


my @files = glob("*.bmp");
my @files2 = glob("*.ico");

foreach $filename ((@files, @files2))
{
   make_xpm($filename);
}


sub make_xpm($)
{
   my $filename = shift(@_);
   my $basename = substr($filename, 0, -4); ## assume filename ends in .bmp
or .ico
   system("convert $filename $basename.xpm");
   system("./xpmname.pl $basename.xpm");
}

   
