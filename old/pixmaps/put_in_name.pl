#!/usr/local/bin/perl -w

my @args = @ARGV;

my $i;


foreach $filename (@args)
{
   put_in_name($filename);
}


sub put_in_name($)
{
   my $filename = shift(@_);
   my $basename = substr($filename, 0, -4); ## assume filename ends in .xpm
   open(F, "< $filename");
   open(F2, "> /tmp/junk");
   while (<F>)
   {
      s/static char/const char/i;
      s/magick/${basename}_xpm/i;
      print F2 $_;
   }
   close(F);
   close(F2);
   system("mv /tmp/junk $filename");
}

   
