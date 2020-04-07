lib_name = libzbxhaproxy.so
lib_dir = /usr/local/lib
zbx_include = /home/user/zabbix/include
gcc_com_options = -std=gnu11 -fPIC -Wall -lm -I$(zbx_include)
gcc = gcc $(gcc_com_options)
build = ./build
src = ./src

.PHONY: clean install

build: build_dir fix_config_h zbxhaproxy

build_dir:
	mkdir -p $(build)

fix_config_h:
	if test ! -f $(zbx_include)/config.h; then echo "" > $(zbx_include)/config.h; fi

zbxhaproxy: hash_table.o prime.o haproxy_stat.o haproxy_servers.o zbxhaproxy.o
	$(gcc) -shared -o $(build)/$(lib_name) $(addprefix $(build)/,$^)

prime.o: $(addprefix $(src)/,prime.c prime.h)
	$(gcc) $< -c -o $(addprefix $(build)/,$@)

hash_table.o: $(addprefix $(src)/,hash_table.c hash_table.h)
	$(gcc) $< -c -o $(addprefix $(build)/,$@)

haproxy_servers.o: $(addprefix $(src)/,haproxy_servers.c haproxy_servers.h)
	$(gcc) $< -c -o $(addprefix $(build)/,$@)

haproxy_stat.o: $(addprefix $(src)/,haproxy_stat.c haproxy_stat.h)
	$(gcc) $< -c -o $(addprefix $(build)/,$@)

zbxhaproxy.o: $(addprefix $(src)/,zbxhaproxy.c zbxhaproxy.h)
	$(gcc) $< -c -o $(addprefix $(build)/,$@) -I$(zbx_include)

install:
	install -Z -v -m 755 $(build)/$(lib_name) $(lib_dir)/$(lib_name)

clean:
	rm -rf $(build)
