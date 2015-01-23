
# EDLIB_REF should be commit hash or branch name.

.PHONY: edlib
edlib:
	mkdir -p edlib
	curl --silent -o edlib/edlib.cpp https://raw.githubusercontent.com/Martinsos/edlib/${EDLIB_REF}/src/edlib.cpp
	curl --silent -o edlib/edlib.h https://raw.githubusercontent.com/Martinsos/edlib/${EDLIB_REF}/src/edlib.h

clean:
	rm -r edlib build node_modules npm-debug.log
