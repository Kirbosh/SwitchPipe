HOST_BUILD_DIR := build/host
HOST_BIN := $(HOST_BUILD_DIR)/switch_newpipe_host
HOST_CXX := g++
HOST_CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Iinclude -Ivendor/third_party
HOST_LDLIBS := -lssl -lcrypto -pthread

.PHONY: host clean-host

host: $(HOST_BIN)

HOST_SRCS := \
	src/common/auth_store.cpp \
	src/common/account_actions.cpp \
	src/common/http_client.cpp \
	src/common/log.cpp \
	src/common/settings_store.cpp \
	src/common/youtube_catalog_service.cpp \
	src/common/youtube_resolver.cpp \
	src/host/throttling_decrypter_host.cpp \
	src/host/main.cpp

$(HOST_BIN): $(HOST_SRCS) include/newpipe/auth_store.hpp include/newpipe/account_actions.hpp include/newpipe/models.hpp include/newpipe/catalog_service.hpp include/newpipe/http_client.hpp include/newpipe/settings_store.hpp include/newpipe/throttling_decrypter.hpp include/newpipe/youtube_catalog_service.hpp include/newpipe/youtube_resolver.hpp
	mkdir -p $(HOST_BUILD_DIR)
	$(HOST_CXX) $(HOST_CXXFLAGS) $(HOST_SRCS) -o $(HOST_BIN) $(HOST_LDLIBS)

clean-host:
	rm -rf $(HOST_BUILD_DIR)
