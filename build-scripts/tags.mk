#
# Makefile for STM32 targets
#	  This file contains targets for generating ctags
#
# Required Environment Vars:
#  - CTAGS : the Ctags program
#  - CTAGS_FLAGS : any additional flags to pass
#  - TAGS_DIR : More tags to include
#
# Copyright (c) Yifei Zhang @ Dexta Robotics
# Nov. 2015
#

TAGS_VIM=tags.vim

TAGS_DIR_CONVERTED=$(foreach d,$(TAGS_DIR),$(realpath $(d))/tags)
.ctagsignore:
	@touch .ctagsignore

.extras.vim:
	@touch extras.vim

tags.vim: Makefile
	@echo "  Generating Vim Script"
	@echo "    Directory Containing tags:"
	@echo "      TAGS_DIR = $(TAGS_DIR)"
	@echo "    Tags File:"
	@for t in $(TAGS_DIR_CONVERTED); do \
		echo "      $$t" ; \
	done
	@echo "\"--* auto-generated *--" > $(TAGS_VIM)
	@echo "\"this is a script for importing tags" >> $(TAGS_VIM)
	@for t in $(TAGS_DIR_CONVERTED); do \
		echo "set tags+=$$t" >> $(TAGS_VIM); \
	done
	@echo "nnoremap <Leader>fl :!bash -c \"make flash\"<CR>" >> $(TAGS_VIM)
	@echo "if filereadable(\"extras.vim\")" >> $(TAGS_VIM);
	@echo "    source extras.vim" >> $(TAGS_VIM);
	@echo "endif" >> $(TAGS_VIM);

build_tags: .ctagsignore .extras.vim
	@echo "  Running Ctags on Current Directory"
	@echo "    CTAGS $(CTAGS_FLAGS) --exclude=@.ctagsignore --extra=+f -R ."
	@$(CTAGS) $(CTAGS_FLAGS) --exclude=@.ctagsignore --extra=+f -R .

clean_tags:
	@rm -f tags
	@rm -f $(TAGS_VIM)

tags: build_tags tags.vim
