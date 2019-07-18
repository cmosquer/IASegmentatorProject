project(ia_segmentador_abdominal)

mitk_create_plugin(
  EXPORT_DIRECTIVE CLIENT_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkPython IASegmentadorAbdominal
)
