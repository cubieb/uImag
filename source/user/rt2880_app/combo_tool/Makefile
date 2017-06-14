base_dir :=combo_loader src

all romfs:
	for i in $(base_dir) ; do make -C $$i $@ || exit $?; done

clean:
	for i in $(base_dir) ; do make -C $$i clean ; done
