PROJECT_VERSION=`./debianize.sh getversion`
DISTPREFIX=`./debianize.sh getproject`
DISTJAR=$(DISTPREFIX)-$(PROJECT_VERSION).jar
JAVAC=javac
JAR=jar

all: build.xml dist/lib/$(DISTJAR)

getname:
	@echo dist/lib/$(DISTJAR)

dist:
	mkdir -p dist

dist/lib/$(DISTJAR): dist
	ant dist

build.xml:
	./antify.sh >$@

clean:
	rm -f build.xml
	rm -f dist/lib/$(DISTPREFIX)*.jar
	rm -rf build
	cd java; make clean

cleanall:	clean
	rm -rf debian
	rm -rf deb
	rm -rf download

debian:
	mkdir debian

deb/%:
	mkdir -p deb
	touch $@
	./debianize.sh create $@

debian/compat:
	echo "7" >$@

debian/%:
	./debianize.sh create $@ >$@

deb:	debian debian/rules debian/control debian/compat debian/changelog deb/javadoc deb/jlibs
	dpkg-buildpackage -uc -us

/usr/bin/emacs:
	sudo apt-get install emacs

emacsdevenv:


.PHONY: clean all cleanall getname deb emacsdevenv work/%  interface/%
