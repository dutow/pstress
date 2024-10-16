function init_pg_tde_only_for_db(sqlconn)
	sqlconn:execute_query("CREATE EXTENSION pg_tde;")
	sqlconn:execute_query("SET default_table_access_method = tde_heap;")
	sqlconn:execute_query("SELECT pg_tde_add_key_provider_file('reg_file', '/tmp/pg_tde_test_keyring.per');")
	sqlconn:execute_query("SELECT pg_tde_set_principal_key('global-principal-key', 'reg_file');")
end
