PYTHON=python

.PHONY: all build install install-user dist check clean test tags

torkel:
	@$(PYTHON) setup.py -q build
	@$(PYTHON) test.py

all: test

test: build
	@rm -rf .tmp
	@mkdir .tmp
	@mkdir .tmp/a
	@mkdir .tmp/b
	@echo test > .tmp/a/testfile
	$(PYTHON) test.py .tmp/a .tmp/b
	@echo -n "Test: "
	@if [ -r .tmp/b/testfile ]; then echo passed! ; else echo failed! ; fi

build:
	$(PYTHON) setup.py build

install: build
	$(PYTHON) setup.py install

install-user: build
	$(PYTHON) setup.py install --user

dist: build
	$(PYTHON) setup.py sdist

check: build
	$(PYTHON) setup.py check

clean:
	$(PYTHON) setup.py clean
	rm -f tags MANIFEST
	rm -rf build/
	rm -rf dist/
	rm -rf .tmp/
	# find -name .csync_journal\* | xargs rm -v

tags:
	ctags csyncmodule.c
