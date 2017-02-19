#
# Makefile for STM32 targets
#
# Required Vars:
#   YCM_FLAGS_TEMPLATE
#   YCM_FLAGS
#

YCM_FLAGS_CONVERTED=$(foreach f,$(YCM_FLAGS),\'$(f)\',)

YCM_EXTRA_CONF=.ycm_extra_conf.py
YCM_EXTRA_CONF_TEMP=$(YCM_EXTRA_CONF).temp

.ycm_extra_conf.py: Makefile
	@rm -f $(YCM_EXTRA_CONF)c
	@echo "  Generating .ycm_extra_conf.py"
	@echo "    Flags To Add:"
	@for f in $(YCM_FLAGS_CONVERTED); do echo "      $$f" ; done
	@echo "# -- auto-generated BEGIN --" > $(YCM_EXTRA_CONF_TEMP)
	@for f in $(YCM_FLAGS_CONVERTED); do \
		echo "$$f" >> $(YCM_EXTRA_CONF_TEMP) ; \
	done
	@echo "# -- auto-generated END   --" >> $(YCM_EXTRA_CONF_TEMP)
	@cat $(YCM_FLAGS_TEMPLATE) | \
		sed -e \
		'/#\[YCM FLAGS\]#/{r $(YCM_EXTRA_CONF_TEMP)' -e '}' \
		> $(YCM_EXTRA_CONF)
	@rm -f $(YCM_EXTRA_CONF_TEMP)

clean_ycm_extra_conf:
	@rm -f $(YCM_EXTRA_CONF)
	@rm -f $(YCM_EXTRA_CONF)c

ycm_extra_conf: Makefile .ycm_extra_conf.py
