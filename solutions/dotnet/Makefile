TOP=$(abspath ../..)
include $(TOP)/defs.mak

APPBUILDER=$(TOP)/scripts/appbuilder
APPNAME=HelloWorld

#ifdef STRACE
#OPTS += --strace
#endif

all: appdir private.pem

run-unsigned:
	$(MYST) mkcpio appdir rootfs
	$(MYST_EXEC) $(OPTS) --memory-size=1024m rootfs /app/HelloWorld

run: appdir private.pem
	$(MYST) package appdir private.pem config.json
	myst/bin/$(APPNAME) $(OPTS)
	
gdb: appdir private.pem
	$(MYST) package appdir private.pem config.json
	$(MYST_GDB) --args myst/bin/$(APPNAME) $(OPTS)

appdir:
	$(APPBUILDER) Dockerfile

private.pem:
	openssl genrsa -out private.pem -3 3072

clean:
	sudo rm -fr appdir rootfs HelloWorld/build HelloWorld/obj HelloWorld/bin myst private.pem
