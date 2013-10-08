if doc

doc: all-local
all-local:
	@echo -n Generating documentation \(this may take a while\)...
	@SRC_DIR=$(top_srcdir) @DOXYGEN@ doxyfile.conf >/dev/null 2>&1
	@echo done
	
	@echo  The documentation is under: $(builddir)/doc/ 

endif
