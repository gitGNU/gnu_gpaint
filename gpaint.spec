# Spec file for gpaint

Summary: Small-scale painting program for GNOME, the GNU Desktop
Name: gpaint
Version: 0.3.0
Release: 1
Copyright: GPL
Group: Applications/Multimedia
Source: http://www.atai.org/gpaint/gpaint-%{ver}.tar.gz
URL: http://www.gnu.org/software/gpaint/
Packager: Pierre Sarrazin <sarrazip@iname.com>
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root
Requires: gtk+ >= 1.2.8, gnome-libs >= 1.2.4, imlib, gdk-pixbuf >= 
0.9.0, gnome-print >= 0.25

%description
This is gpaint (GNU Paint), a small-scale painting program for GNOME, 
the GNU
Desktop.  Gpaint does not attempt to compete with GIMP.  Think of GIMP 
is like
Photoshop as gpaint is like Windows Paint.



%prep
%setup

%build
./configure --prefix=/usr && make

%install
make DESTDIR="$RPM_BUILD_ROOT" install

%files
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog NEWS README THANKS TODO
%{prefix}/bin/*
%{prefix}/share/pixmaps/*



