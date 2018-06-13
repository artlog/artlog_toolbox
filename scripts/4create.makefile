PACKAGE?=org.artisanlogiciel.games
MAIN_CLASS?=Main
PACKAGE_DIR=$(subst .,/,$(PACKAGE))
OUT=out
EDITOR=emacs

$(OUT):
	mkdir -p $(OUT)

clean:
	@find $(PACKAGE_DIR) -name "*.class" -type f -print0|xargs -0 rm 2>/dev/null && echo "cleaned classes in source"
	@find $(OUT) -name "*.class" -type f -print0|xargs -0 rm 2>/dev/null || echo "nothing to clean"

test:
	javac -d $(OUT) $(PACKAGE_DIR)/$(MAIN_CLASS).java
	java -cp $(OUT) $(PACKAGE).$(MAIN_CLASS)


run/%:	$(OUT)
	javac -d $(OUT) $(PACKAGE_DIR)/$(subst run/,,$@).java
	java -cp $(OUT) $(PACKAGE)/$(subst run/,,$@) 

compile/%:
	javac -d $(OUT) $(PACKAGE_DIR)/$(subst compile/,,$@).java

$(PACKAGE_DIR)/%.java:
	./generate_new.sh class $(subst .java,,$(subst $(PACKAGE_DIR)/,,$@))

interface/%:
	./generate_new.sh interface package_dir=$(PACKAGE_DIR) $(subst interface/,,$@)
	$(EDITOR) $(PACKAGE_DIR)/$(subst interface/,,$@).java

work/%:	$(PACKAGE_DIR)/$(subst work/,,%).java
	$(EDITOR) $<

work:	work/$(MAIN_CLASS)

save:	
	git citool

.PHONY: clean test work work/% run/% save compile/% interface/%

# tried to avoid intermediate file removal : does not work
# .SECONDARY: $(PACKAGE_DIR)/%.java 

# this does work : once precious intermediate file is not removed.
.PRECIOUS: $(PACKAGE_DIR)/%.java 
