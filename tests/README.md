
# termfu tests


## Running a test

Build and run a test as follows from the root directory...

```bash
$ make test_gdb t=tests/test_gdb_1.c

$ make test_pdb t=tests/test_pdb_1.c
```
<br>

These tests build off of the `../obj/dev` object files. Each test's `main()` object file is also located there. The test binary is `../termfu_test`. Running the above command will rebuild any changes made to the `../src/*` files. See `../Makefile` for more details.
<br>

Adjust `../configs/.termfu_test_gdb` and `../configs/.termfu_test_gdb` as needed but do not include these changes in a PR.
<br>

