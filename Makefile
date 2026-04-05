#     _      _       _     _      ____   _____ 
#    | |    (_)     | |   | |    / __ \ / ____|
#    | |     _  __ _| |__ | |_  | |  | | (___  
#    | |    | |/ _` | '_ \| __| | |  | |\___ \ 
#    | |____| | (_| | | | | |_  | |__| |____) |
#    |______|_|\__, |_| |_|\__|  \____/|_____/ 
#               __/ |                          
#              |___/    
#    ____ ___  _   _      ____  _     ____ ___  _      _  ____ 
#   /  _ \\  \//  / \__/|/  _ \/ \ |\/  _ \\  \//     / \/  _ \
#   | | // \  /   | |\/||| / \|| | //| / \| \  /_____ | || | \|
#   | |_\\ / /    | |  ||| |-||| \// | \_/| /  \\____\| || |_/|
#   \____//_/     \_/  \|\_/ \|\__/  \____//__/\\     \_/\____/
#                                                              

.PHONY: all
all:
	@echo "--- Light OS Help ---"
	@echo ""
	@echo "make build - Build Light OS | Before start write .make utils."
	@echo "make clean - Clear build files in Light OS"
	@echo "make utils - Downloads utils for build Light OS"
	@# >>> Dont need to :> |
	@#echo "make todo - Show TODO List with Light OS"

.PHONY: build
build:
	@echo "--- Light OS ---"
	chmod +x .light.sh && ./.light.sh
	@# >>> Fix Errors with build (If have) 

.PHONY: clean
clean:
	rm -rf root bin cross tag
	rm -f tags
	@echo "Cleared."
	
.PHONY: utils
utils:
	@echo "--- Build Utils for Light OS ---"
	chmod +x util/.utils.sh && ./util/.utils.sh
	@# >>> Only for Build Utils, You can open :> ^^^
	
.PHONY: todo
todo:
	chmod +x util/.todo.sh && ./util/.todo.sh
	@# >>> It's to make TODO List, Ignore this :) This for Mavox-ID
	@# >>> This is all done by me, but maybe I won't do all of this
	@# >>> If you're reading this, Light OS is already running, but I'm probably going to go crazy before I get all the TODOs done.
	@# >>> If you need all the TODOs and you are an enthusiast and developer, then when you run make todo, it will appear in the TODO info file.
