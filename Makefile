MODULES = jsonb_delete_array
EXTENSION = jsonb_delete_array
DATA = jsonb_delete_array--1.0.sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
