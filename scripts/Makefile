PROJECT_VERSION=`./debianize.sh getversion`
DISTPREFIX=artloglaby
DISTJAR=$(DISTPREFIX)-$(PROJECT_VERSION).jar
JDEE_VERSION=2.4.1
JAVAC=javac
JAR=jar
RHINO_VER=1_7R5

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
	@( [ -d ~/.emacs.d/jdee-$(JDEE_VERSION) ] && echo "DON'T remove installed ~/.emacs.d/jdee-$(JDEE_VERSION). To remove jdee do 'make removejdee'" ) || true

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

download/jdee-bin-$(JDEE_VERSION).tar.bz2:
	mkdir -p download; cd download; wget http://sourceforge.net/projects/jdee/files/jdee/$(JDEE_VERSION)/jdee-bin-$(JDEE_VERSION).tar.bz2

~/.emacs.d/jdee-$(JDEE_VERSION): /usr/bin/emacs download/jdee-bin-$(JDEE_VERSION).tar.bz2
	mkdir -p ~/.emacs.d; tar -xjf download/jdee-bin-$(JDEE_VERSION).tar.bz2 -C ~/.emacs.d
	echo "(add-to-list 'load-path \"~/.emacs.d/jdee-$(JDEE_VERSION)/lisp\")\n(load \"jde\")" >>~/.emacs.d/init.el

emacsdevenv: ~/.emacs.d/jdee-$(JDEE_VERSION)

removejdee:
	rm -rf ~/.emacs.d/jdee-$(JDEE_VERSION)
	sed -i -n -e "/^(add-to-list 'load-path \"~\/\.emacs\.d\/jdee-$(JDEE_VERSION)\/lisp\")$$/d" -e "/^(load \"jde\")$$/d" -e"/^.*$$/p" ~/.emacs.d/init.el

download/rhino$(RHINO_VER).zip:
	mkdir -p download
	cd download; wget https://github.com/mozilla/rhino/releases/download/Rhino$(RHINO_VER)_RELEASE/rhino$(RHINO_VER).zip

.PHONY: clean all cleanall getname deb emacsdevenv removejdee
