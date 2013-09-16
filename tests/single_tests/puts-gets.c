#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "mdhim.h"
#include "db_options.h"

int main(int argc, char **argv) {
	int ret;
	int provided = 0;
	struct mdhim_t *md;
	char *key;
	int value;
	struct mdhim_rm_t *rm;
	struct mdhim_getrm_t *grm;
        db_options_t *db_opts;
	int i;

	ret = MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
	if (ret != MPI_SUCCESS) {
		printf("Error initializing MPI with threads\n");
		exit(1);
	}

	if (provided != MPI_THREAD_MULTIPLE) {
                printf("Not able to enable MPI_THREAD_MULTIPLE mode\n");
                exit(1);
        }
        
        db_opts = db_options_init();
        db_options_set_path(db_opts, "./");
        db_options_set_name(db_opts, "mdhimTstDB");
        db_options_set_type(db_opts, 2); // type = 2 (LevelDB)
        db_options_set_key_type(db_opts, MDHIM_STRING_KEY); 
	db_options_set_debug_level(db_opts, MLOG_DBG);

	md = mdhimInit(MPI_COMM_WORLD, db_opts);
	if (!md) {
	  printf("Error initializing MDHIM\n");
	  exit(1);
	}	

	//Put the keys and values
	for (i = 0; i < 2; i++) {
		key = malloc(100);
		sprintf(key, "%c", (int) '0' + (md->mdhim_rank + 1) + i);
		value = 500 * (md->mdhim_rank + 1) + i;
		rm = mdhimPut(md, key, strlen(key) + 1, 
			      &value, sizeof(value));
		if (!rm || rm->error) {
			printf("Error inserting key/value into MDHIM\n");
		} else {
			printf("Successfully inserted key/value into MDHIM\n");
		}

		//Commit the database
		ret = mdhimCommit(md);
		if (ret != MDHIM_SUCCESS) {
			printf("Error committing MDHIM database\n");
		} else {
			printf("Committed MDHIM database\n");
		}

		//Get the values
		value = 0;
		grm = mdhimGet(md, key, strlen(key) + 1, MDHIM_GET_EQ);
		if (!grm || grm->error) {
			printf("Error getting value for key: %s from MDHIM\n", key);
		} else {
			printf("Successfully got value: %d from MDHIM for key: %s\n", *((int *) grm->value), key);
		}

		free(key);
	}
	ret = mdhimClose(md);
	if (ret != MDHIM_SUCCESS) {
		printf("Error closing MDHIM\n");
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	return 0;
}
