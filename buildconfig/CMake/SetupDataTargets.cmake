# ######################################################################################################################
# Define targets to download the data
# ######################################################################################################################
include(MantidExternalData)

# None of our tests reference files directly as arguments so we have to manually call ExternalData_Expand_Arguments to
# register the files with the ExternalData mechanism
function(_create_data_target _targetname _content_link_patterns)
  get_filename_component(EXTERNALDATATEST_SOURCE_DIR ${PROJECT_SOURCE_DIR} ABSOLUTE)
  foreach(_pattern ${_content_link_patterns})
    file(
      GLOB_RECURSE _content_links
      RELATIVE "${EXTERNALDATATEST_SOURCE_DIR}"
      ${_pattern}
    )
    foreach(link ${_content_links})
      string(REGEX REPLACE "\\.md5$" "" link ${link})
      externaldata_expand_arguments(${_targetname} link_location DATA{${link}})
    endforeach()
  endforeach()
  # Create target to download data from the StandardTestData group.  This must come after all tests have been added that
  # reference the group, so we put it last.
  externaldata_add_target(${_targetname})
  set_target_properties(${_targetname} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endfunction()

# We'll create three targets: - StandardTestData: includes both data required by the unit tests and documentation tests
# - broken down into UnitTestData and DocTestData SystemTestData: data required for the system tests
_create_data_target(UnitTestData "Testing/Data/UnitTest/*.md5")
_create_data_target(DocTestData "Testing/Data/DocTest/*.md5")
_create_data_target(SystemTestData "Testing/Data/SystemTest/*.md5;Testing/SystemTests/tests/framework/reference/*.md5")
# Combine individual targets
add_custom_target(StandardTestData DEPENDS UnitTestData DocTestData)
set_target_properties(StandardTestData PROPERTIES EXCLUDE_FROM_ALL TRUE)
