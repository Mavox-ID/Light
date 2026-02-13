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
	@echo "make build - Build Light OS"
	@echo "make clean - Clear build files in Light OS"

.PHONY: build
build:
	@echo "--- Light OS ---"
	chmod +x .light.sh && ./.light.sh

.PHONY: clean
clean:
	rm -rf root bin && rm tags
	@echo -n "Delete C++ (cross)? [y/N]: " && read ans && \
	if [ "$$ans" = "y" ] || [ "$$ans" = "Y" ] || [ "$$ans" = "д" ] || [ "$$ans" = "Д" ]; then \
		rm -rf cross; \
		echo "Folder 'cross' deleted."; \
	else \
		echo "Skipping..."; \
	fi
	@echo "Cleared."
