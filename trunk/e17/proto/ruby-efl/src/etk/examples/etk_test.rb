#------------------------------------RUBY ETK_TEST --------------------------------------------
# This is a rewrite in ruby of the C program etk_test that is found in etk cvs. 
# It is not implemented fully and it is not guaranteed to work at any moment, because
# it might take a while to sync it after API changes (it's not autogenerated like the bindings)
#
# See the other files in this directory for the actual widget examples, this is just the setup.
#----------------------------------------------------------------------------------------------

require 'dl/ffcall-callback'
require 'ruby-efl/etk/ruby-etk'

# Uncomment these if you want to see a lot of debug information in stdout
# CClass.debug = true
# CClass.debug_level = [:call]

include Etk

PACKAGE_DATA_DIR = '/opt/e17/share/etk'

class Example < Window
    attr_accessor :label
    
    def initialize()
        @label = self.class.to_s.gsub(/Example$/,'')
        super()
    end
end

require File.dirname(File.expand_path(__FILE__)) + '/etk_test_tree.rb'
require File.dirname(File.expand_path(__FILE__)) + '/etk_test_button.rb'

class EtkTestWindow < Window
    
    NUM_COLS = 2
    CATEGORIES = [
        { :title => "Basic Widget", :examples => [ 
            ButtonExample
            ]
        } ,
        { :title => "Advanced Widget" , :examples => [] },
        { :title => "Container" , :examples => [
            TreeExample
            ] 
        },
        { :title => "Dialog" , :examples => [] },
        { :title => "Misc" , :examples => [] }
    ]
    
    def initialize
        super() 
        self.title = "Etk Test Application"
        self.border_width = 5
        self << Etk::DestroyedSignal.new() { |*args| puts "main win closed"; EtkBase.main_quit  } 
        
        box = VBox.new(false, 0)
        self << box
        
        CATEGORIES.each { |category|
            frame = Frame.new(category[:title])
            box.append(frame, Etk::BOX_START, Etk::BOX_NONE, 0)

            table = Table.new(NUM_COLS, (category[:examples].length + NUM_COLS - 1) / NUM_COLS, true);
            frame << table
            
            category[:examples].each_with_index { |example, i| 
                example = example.new
                button = Button.new
                button.label = example.label
                button << ButtonClickedSignal.new { example.run if example.respond_to?(:run) }
                
                table.attach_default(button, i % NUM_COLS, i % NUM_COLS, i / NUM_COLS, i / NUM_COLS)
            }
        }
        
        show_all
    end
end

EtkBase.init(0, nil)
ToolTips.enable
w = EtkTestWindow.new
EtkBase.main
