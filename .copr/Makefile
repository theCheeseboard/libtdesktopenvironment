srpm:
	dnf -y install git rpkg
	git submodule init
	git submodule update
	rpkg --config dist/rpkg.conf srpm --spec $(spec) --outdir $(outdir)
