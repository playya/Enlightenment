.PHONY: all install detourious.edj

all: detourious.edj

# XXX shouldn't be phony, but managing all dependence edc is a pain
detourious.edj:
	edje_cc -id . -fd . main.edc -o detourious.edj

install: detourious.edj
	rm -f ~/.e/e/themes/detourious.edj
	cp detourious.edj ~/.e/e/themes

illume:
	edje_cc -DBUILD_ILLUME=1 -DUSE_BOLD_FONT=1 -id . -fd . main.edc -o dtrs-illume.edj

dark:
	edje_cc -DCOLORS=1 -id . -fd . main.edc -o dtrs-dark.edj

illume-mine:
	edje_cc -DILLUME_KBD_TERMINAL_ONLY=1 -DBUILD_ILLUME=1 -DUSE_BOLD_FONT=1 -id . -fd . main.edc -o dtrs-illume.edj

