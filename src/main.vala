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


extern Resource gpaint_get_resource();

namespace Gpaint
{

    
    public class App : Gtk.Application
    {        
        private static const string resource_prefix = "/org/gnu/gpaint/";
        private static const string ui_file = resource_prefix + "gpaint.ui";
        public static const string app_name = "gpaint";
        public static const string app_id = "org.gnu." + app_name;
        private Resource resources;
        private const GLib.ActionEntry[] actions = 
        {
            { "action_new", on_new },
            { "action_about", on_about },
            { "action_quit", on_quit }
            
        };
        Builder builder;
        
       
        private void on_new(SimpleAction action, GLib.Variant? parameter) 
        {
            create_new_document();
        }
        
        
        private void on_about(SimpleAction action, GLib.Variant? parameter) 
        {
            GLib.message("on_about!");
            
        }
        private void on_quit(SimpleAction action, GLib.Variant? parameter) 
        {
            Gtk.main_quit();
        }
        
        Window create_new_document()
        {
            builder = new Builder();	
            try 
            {
                
                builder.add_from_resource(ui_file);
            }
            catch (GLib.Error err) 
            {
                GLib.critical("Failed to add builder UI to resource, %s", err.message);    
            }
            builder.connect_signals(null);
            Gtk.ApplicationWindow window = (Gtk.ApplicationWindow) builder.get_object("main_window") ;
            if (window == null) 
            {
                GLib.critical("Cannot build gpaint main window!");    
            }
           
            add_window(window);
            window.destroy.connect(Gtk.main_quit);
            //window.show_all();
            window.show();
            
            return window;
            
        }
        
        public App() 
        {          
            Object(application_id: app_name, flags: ApplicationFlags.FLAGS_NONE);
        }
        
        protected override void startup()
        {
            base.startup();
            resources = gpaint_get_resource();

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

