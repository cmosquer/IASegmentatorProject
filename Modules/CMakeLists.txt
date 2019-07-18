set(module_dirs
  IASegmentadorAbdominal
)

foreach(module_dir ${module_dirs})
  add_subdirectory(${module_dir})
endforeach()
