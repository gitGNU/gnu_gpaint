/*
	Copyright 2013, 2014 Li-Cheng (Andy) Tai
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


namespace Gpaint
{

    public class App : Gtk.Application
    {        
        private static const string ui_file = "gpaint.ui";
        private const GLib.ActionEntry[] actions = 
        {
            { "action_new", on_new },
            { "action_quit", on_quit }
            
        };
        Builder builder;
        private void on_new(SimpleAction action, GLib.Variant? parameter) 
        {
            
        }
        
        private void on_quit(SimpleAction action, GLib.Variant? parameter) 
        {
            Gtk.main_quit();
        }
        Window create_new_document()
        {
            try
            {
                builder.add_from_file(ui_file);
                builder.connect_signals(null);
                Gtk.Window window = builder.get_object("main_window") as Gtk.Window;
                window.@set("application", this);    /* hack, to make window a GtkApplicationWindow*/
                window.destroy.connect(Gtk.main_quit);
            
                //window.show_all();
                window.show();
                
                return window;
            } 
            catch (GLib.Error err)
            {
                stdout.printf("failure in created new document, %s\n", err.message);
            }
            return (ApplicationWindow) null;
        }
        
        public App() 
        {            
        }
        
        protected override void startup()
        {
            base.startup();
            builder = new Builder();	

            add_action_entries(actions, this);            
            
        }
        
        protected override void activate()
        {
            create_new_document();	
        }
    }
    
    /* main creates and runs the application. */
    public static int main (string[] args) {
        App app = new App();
        
        return app.run(args);
    }
}

