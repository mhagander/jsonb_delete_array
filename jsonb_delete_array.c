#include "postgres.h"
#include "catalog/pg_type.h"
#include "utils/jsonb.h"
#include "utils/jsonapi.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(jsonb_delete_array);

Datum
jsonb_delete_array(PG_FUNCTION_ARGS)
{
	Jsonb	   *in = PG_GETARG_JSONB(0);
	ArrayType  *keys = PG_GETARG_ARRAYTYPE_P(1);
	Datum	   *keys_elems;
	bool	   *keys_nulls;
	int			keys_len;
	JsonbParseState *state = NULL;
	JsonbIterator *it;
	JsonbValue	v,
			   *res = NULL;
	bool		skipNested = false;
	JsonbIteratorToken r;

	if (ARR_NDIM(keys) > 1)
		ereport(ERROR,
				(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
				 errmsg("wrong number of array subscripts")));

	if (JB_ROOT_IS_SCALAR(in))
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("cannot delete from scalar")));

	if (JB_ROOT_COUNT(in) == 0)
		PG_RETURN_JSONB(in);

	deconstruct_array(keys, TEXTOID, -1, false, 'i',
					  &keys_elems, &keys_nulls, &keys_len);

	if (keys_len == 0)
		PG_RETURN_JSONB(in);

	it = JsonbIteratorInit(&in->root);

	while ((r = JsonbIteratorNext(&it, &v, skipNested)) != 0)
	{
		skipNested = true;

		if ((r == WJB_ELEM || r == WJB_KEY) && v.type == jbvString)
		{
			int i;
			bool found = false;
			for (i = 0; i < keys_len; i++)
			{
				char *keyptr;
				int keylen;
				if (keys_nulls[i])
					continue;
				keyptr = VARDATA_ANY(keys_elems[i]);
				keylen = VARSIZE_ANY_EXHDR(keys_elems[i]);
				if (keylen == v.val.string.len &&
					memcmp(keyptr, v.val.string.val, keylen) == 0)
				{
					found = true;
					break;
				}
			}
			if (found)
			{
				/* skip corresponding value as well */
				if (r == WJB_KEY)
					JsonbIteratorNext(&it, &v, true);

				continue;
			}
		}

		res = pushJsonbValue(&state, r, r < WJB_BEGIN_ARRAY ? &v : NULL);
	}

	Assert(res != NULL);

	PG_RETURN_JSONB(JsonbValueToJsonb(res));
}
