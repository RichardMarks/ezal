# EZAL Makefile
# Compiles ezal.c to ezal.o
# You need to link ezal.o with your game .o files
# and use the include flag to find the ezal.h file
# eg if you put ezal.h into ./ezal/
# then use -Iezal

# You will need these LDFLAGS to link your game code
# LDFLAGS ?= $(shell pkg-config \
# 	allegro-5 allegro_main-5 allegro_font-5 allegro_ttf-5 \
# 	allegro_image-5 allegro_audio-5 allegro_acodec-5 --libs)
# for example:
# gcc game.o ezal.o -o coolest-game-ever $(LDFLAGS)

CC := gcc
CFLAGS ?= -DDEBUG -O0 -MMD -MP -g $(shell pkg-config \
	allegro-5 allegro_main-5 allegro_font-5 allegro_ttf-5 \
	allegro_image-5 allegro_audio-5 allegro_acodec-5 --cflags)
.PHONY: clean
.PHONY: install
.PHONY: uninstall
libezal.a: ezal.o
	@echo "Creating EZAL Static Library"
	@ar -rc $@ $^
	@ranlib $@
ezal.o: ezal.c
	@echo "Compiling EZAL Source"
	@$(CC) -c $^ -o $@ $(CFLAGS)
clean:
	@echo "Cleaning EZAL Project"
	@$(RM) ezal.o libezal.a ezal.d
install:
	@echo "Installing EZAL"
	@mkdir -p ~/ezal/include
	@mkdir -p ~/ezal/lib
	@cp ezal.h ~/ezal/include/
	@cp ezal.o ~/ezal/lib/
	@cp libezal.a ~/ezal/lib/
uninstall:
	@echo "Uninstalling EZAL"
	@$(RM) -r ~/ezal
