@default: build install
version :=`rg -o "(\d+)\.(\d+).(\d+)" metadata.json`

[working-directory: 'build']
install:
	mkdir -p $HOME/.local/lib/albert/
	cp bin/contacts_kde.so $HOME/.local/lib/albert/

@uninstall:
	rm  $HOME/.local/lib/albert/contacts_kde.so


[working-directory: 'build']
@build: init
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .. -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/
	cmake --build . -j$(nproc)
	make

@init:
	[ ! -d build ] && mkdir build/ || exit 0

@clean: uninstall
	rm -rf build/

@release:
    git tag {{version}}
    git-chglog >| CHANGELOG.md
    git add CHANGELOG.md
    git commit -m "doc: CHANGELOG {{version}}"

