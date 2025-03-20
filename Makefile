# Name of your application
APPLICATION = CoAPServer
# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../..

# Include packages that pull up and auto-init the link layer.
USEMODULE += auto_init_gnrc_netif
USEMODULE += netdev_default
USEMODULE += gnrc_icmpv6_echo
# Include the shell package for accessing the device over the command line
USEMODULE += shell
# Include the shell commands
USEMODULE += shell_cmds_default
# Include xtimer for timeouts
USEMODULE += ps
USEMODULE += xtimer
# Include CoAP functionality
USEMODULE += nanocoap_sock
# Include IPv6 functionality
USEMODULE += gnrc_ipv6_default
USEMODULE += nanocoap_server
USEMODULE += nanocoap_server_observe
USEMODULE += saul_default

DEVELHELP ?= 1


# Change this to 0 to enable code in RIOT that uses parts of the C99 standard
# that are not supported by MSVC:
WERROR ?= 1

include $(RIOTBASE)/Makefile.include


DEFAULT_CHANNEL ?= 16

include $(RIOTMAKE)/default-radio-settings.inc.mk