PYTHON=python

.PHONY: all build install install-user dist check clean test tags

all: test

test: build
	@rm -rf .tmp
	@mkdir .tmp
	@mkdir .tmp/a
	@mkdir .tmp/b
	@echo test > .tmp/a/testfile
	$(PYTHON) test.py .tmp/a .tmp/b
	@test -r .tmp/b/testfile && echo Sucess!
	@test -r .tmp/b/testfile || echo Test failed!

build:
	$(PYTHON) setup.py build
#	$(PYTHON) setup.py -q build

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

tags:
	ctags csyncmodule.c
