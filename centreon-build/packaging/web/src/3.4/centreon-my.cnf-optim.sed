{
/\[mysqld\]/a\
##### OPTIMIZE FOR CENTREON #####\
innodb_file_per_table\
\
open-files-limit = 32000\
\
key_buffer_size = 256M\
sort_buffer_size = 32M\
join_buffer_size = 4M\
thread_cache_size = 64\
read_buffer_size = 512K\
read_rnd_buffer_size = 256K\
max_allowed_packet = 8M\
\
# For 4 Go Ram\
#innodb_additional_mem_pool_size=512M\
#innodb_buffer_pool_size=512M\
\
# For 8 Go Ram\
#innodb_additional_mem_pool_size=1G\
#innodb_buffer_pool_size=1G\
#################################
}
