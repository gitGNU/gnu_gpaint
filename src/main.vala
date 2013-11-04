/*
	Copyright 2013 Li-Cheng (Andy) Tai
                      atai@atai.org
                      
	gpaint is free software: you can redistribute it and/or modify it
	under the terms of the GNU General Public License as published by the Free
	Software Foundation, either version 3 of the License, or (at your option)
	any later version.

	gpaint is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
	FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
	more details.

	You should have received a copy of the GNU General Public License along with
	gclip_select. If not, see http://www.gnu.org/licenses/.


*/

using GLib;
using Pango;
using Gtk;
using Gee;

const string ui_file = "gpaint.ui";

namespace Gpaint
{

Builder builder;
Window create_new_document()
{
	builder.add_from_file(ui_file);
	builder.connect_signals(null);
	Gtk.Window window = builder.get_object("main_window") as Gtk.Window;
    	
	window.destroy.connect(Gtk.main_quit);

	window.show_all();
    return window;
    
}

int main(string[] args)
{
    builder = new Builder();	
    
	Gtk.init(ref args);
	
	create_new_document();	
	
	Gtk.main();
	
	return 0;
	
}
}

