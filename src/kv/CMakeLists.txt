include_directories(${CMAKE_CURRENT_SOURCE_DIR})

collect_sources(KV)

add_library(kv INTERFACE)

# When a target links against the interface library `kv`, it will receive a compile-time definition
# so code can check for its presence.
target_compile_definitions(kv INTERFACE SAUNAFS_HAVE_KV)

create_unittest(kv ${KV_TESTS})
link_unittest(kv kv sfscommon)
