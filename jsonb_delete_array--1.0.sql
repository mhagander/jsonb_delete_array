CREATE OR REPLACE FUNCTION jsonb_delete_array(jsonb, text[])
 RETURNS jsonb
 LANGUAGE c
AS '$libdir/jsonb_delete_array', 'jsonb_delete_array';

CREATE OPERATOR - (
 PROCEDURE = jsonb_delete_array,
 LEFTARG = jsonb,
 RIGHTARG = text[]
);
