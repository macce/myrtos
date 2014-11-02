###############################################################################
# RTOS Miscellaneous Build Support Makefile Fragment
###############################################################################

# Make terminal printouts bold.
BEGIN_BOLD := "\033[1m"
END_BOLD := "\033[0m"

# Printout commands:
PRINTOUT_COMPILE := $(BEGIN_BOLD)Compiling $(<F)$(END_BOLD)
