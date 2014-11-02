###############################################################################
# RTOS Default Rules Makefile Fragment
###############################################################################

# Put an empty rule for "all" here, before the rule for "conf_gen" so that an
# simple "make" will make "all" instead of "conf_gen".
.PHONY: all
all:

# Many components do not implement a config generator, so provide a default,
# empty rule for it in the Makefile. That way it will be ok to say
# "make conf_gen" in all components.
.PHONY:	conf_gen
conf_gen:
