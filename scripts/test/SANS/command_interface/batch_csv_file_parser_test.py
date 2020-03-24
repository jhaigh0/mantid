# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)

import os
import unittest

import six

import mantid
from mantid.py3compat import csv_open_type, mock
from sans.command_interface.batch_csv_parser import BatchCsvParser
from sans.common.constants import ALL_PERIODS
from sans.gui_logic.models.RowEntries import RowEntries


class BatchCsvParserTest(unittest.TestCase):
    @staticmethod
    def _save_to_csv(content):
        test_file_path = os.path.join(mantid.config.getString('defaultsave.directory'), 'sans_batch_test_file.csv')
        BatchCsvParserTest._remove_csv(test_file_path)

        with open(test_file_path, 'w') as f:
            f.write(content)
        return test_file_path

    @staticmethod
    def _remove_csv(test_file_path):
        if os.path.exists(test_file_path):
            os.remove(test_file_path)

    def test_that_raises_when_unknown_keyword_is_used(self):
        content = "# MANTID_BATCH_FILE add more text here\n" \
                   "sample_sans,74044,output_as,test,new_key_word,test\n"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()
        with self.assertRaises(KeyError):
            parser.parse_batch_file(batch_file_path)
        BatchCsvParserTest._remove_csv(batch_file_path)

    def test_raises_if_the_batch_file_uses_key_as_val(self):
        content = "# MANTID_BATCH_FILE add more text here\n" \
                   "sample_sans,sample_trans,74024,sample_direct_beam,74014,can_sans,74019,can_trans,74020," \
                   "can_direct_beam,output_as, first_eim\n"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()
        with self.assertRaises(KeyError):
            parser.parse_batch_file(batch_file_path)
        BatchCsvParserTest._remove_csv(batch_file_path)

    def test_that_raises_when_sample_scatter_is_missing(self):
        content = "# MANTID_BATCH_FILE add more text here\n" \
                   "sample_sans,,output_as,test_file\n"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()
        with self.assertRaises(ValueError):
            parser.parse_batch_file(batch_file_path)
        BatchCsvParserTest._remove_csv(batch_file_path)

    def test_that_does_not_raise_when_output_is_missing(self):
        content = "# MANTID_BATCH_FILE add more text here\n" \
                   "sample_sans,test,output_as,\n"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()
        try:
            parser.parse_batch_file(batch_file_path)
        except RuntimeError as e:
            self.fail("Batch files are not required to contain output names as these can be autogenerated. "
                      "Therefore we did not expect a RuntimeError to be raised when parsing batch file without an "
                      "output name. Error raised was: {}".format(str(e)))
        BatchCsvParserTest._remove_csv(batch_file_path)

    def test_that_raises_when_sample_transmission_is_specified_incompletely(self):
        content = "# MANTID_BATCH_FILE add more text here\n" \
                   "sample_sans,test,output_as,test, sample_trans,test, sample_direct_beam,\n"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()
        with self.assertRaises(ValueError):
            parser.parse_batch_file(batch_file_path)
        BatchCsvParserTest._remove_csv(batch_file_path)

    def test_that_raises_when_can_transmission_is_specified_incompletely(self):
        content = "# MANTID_BATCH_FILE add more text here\n" \
                   "sample_sans,test,output_as,test, can_trans,, can_direct_beam, test\n"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()
        with self.assertRaises(ValueError):
            parser.parse_batch_file(batch_file_path)
        BatchCsvParserTest._remove_csv(batch_file_path)

    def test_that_raises_when_can_transmission_is_specified_but_no_can_scatter(self):
        content = "# MANTID_BATCH_FILE add more text here\n" \
                   "sample_sans,test,output_as,test, can_trans,, can_direct_beam, test\n"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()
        with self.assertRaises(ValueError):
            parser.parse_batch_file(batch_file_path)
        BatchCsvParserTest._remove_csv(batch_file_path)

    def test_that_parses_two_lines_correctly(self):
        content = "# MANTID_BATCH_FILE add more text here\n" \
                   "sample_sans,1,sample_trans,2,sample_direct_beam,3,output_as,test_file,user_file,user_test_file\n" \
                   "sample_sans,1,can_sans,2,output_as,test_file2\n"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()

        # Act
        output = parser.parse_batch_file(batch_file_path)

        # Assert
        self.assertEqual(len(output),  2)

        first_line = output[0]
        self.assertEqual(first_line.sample_scatter, "1")
        self.assertEqual(first_line.sample_scatter_period, ALL_PERIODS)
        self.assertEqual(first_line.sample_transmission, "2")
        self.assertEqual(first_line.sample_transmission_period, ALL_PERIODS)
        self.assertEqual(first_line.sample_direct, "3")
        self.assertEqual(first_line.sample_direct_period, ALL_PERIODS)
        self.assertEqual(first_line.output_name, "test_file")
        self.assertEqual(first_line.user_file, "user_test_file")

        second_line = output[1]
        self.assertEqual(second_line.sample_scatter, "1")
        self.assertEqual(second_line.sample_scatter_period, ALL_PERIODS)
        self.assertEqual(second_line.can_scatter, "2")
        self.assertEqual(second_line.can_scatter_period, ALL_PERIODS)
        self.assertEqual(second_line.output_name, "test_file2")

        BatchCsvParserTest._remove_csv(batch_file_path)

    def test_that_parses_period_selection(self):
        content = "# MANTID_BATCH_FILE add more text here\n" \
                   "sample_sans,1p7,can_sans,2P3,output_as,test_file2\n"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()

        # Act
        output = parser.parse_batch_file(batch_file_path)

        # Assert
        self.assertEqual(len(output),  1)

        first_line = output[0]
        self.assertEqual(first_line.sample_scatter, "1")
        self.assertEqual(first_line.sample_scatter_period, 7)
        self.assertEqual(first_line.can_scatter, "2")
        self.assertEqual(first_line.can_scatter_period, 3)
        self.assertEqual(first_line.output_name, "test_file2")

        BatchCsvParserTest._remove_csv(batch_file_path)

    def test_that_does_not_return_excluded_keywords(self):
        content = "# MANTID_BATCH_FILE add more text here\n" \
                   "sample_sans,1,sample_trans,2,sample_direct_beam,3,output_as,test_file,user_file,user_test_file\n" \
                   "sample_sans,1,can_sans,2,output_as,test_file2,"","", background_sans, background\n"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()

        # Act
        output = parser.parse_batch_file(batch_file_path)

        # Assert
        self.assertEqual(len(output),  2)

        first_line = output[0]
        self.assertEqual(first_line.sample_scatter, "1")
        self.assertEqual(first_line.sample_scatter_period, ALL_PERIODS)
        self.assertEqual(first_line.sample_transmission, "2")
        self.assertEqual(first_line.sample_transmission_period, ALL_PERIODS)
        self.assertEqual(first_line.sample_direct, "3")
        self.assertEqual(first_line.sample_direct_period, ALL_PERIODS)
        self.assertEqual(first_line.output_name, "test_file")
        self.assertEqual(first_line.user_file, "user_test_file")

        second_line = output[1]
        # Should have 3 user specified entries and 2 period entries
        self.assertEqual(second_line.sample_scatter, "1")
        self.assertEqual(second_line.sample_scatter_period, ALL_PERIODS)
        self.assertEqual(second_line.can_scatter, "2")
        self.assertEqual(second_line.can_scatter_period, ALL_PERIODS)
        self.assertEqual(second_line.output_name, "test_file2")

        BatchCsvParserTest._remove_csv(batch_file_path)

    def test_can_parse_file_with_empty_final_column(self):
        """There was a bug where certain batch files with an empty final column (no user file provided)
        would not be treated as a column and therefore the batch file would be read as having only 15
        rows.
        We now add an empty final row in the parser if there are 15 rows and the last row provided is
        a batch file key"""
        batch_file_row = ["sample_sans", "1", "sample_trans", "", "sample_direct_beam", "",
                          "can_sans", "", "can_trans", "", "can_direct_beam", "", "output_as", "", "user_file"]
        parser = BatchCsvParser()
        parser._parse_csv_row(batch_file_row, 0)

    def test_bare_comment_without_hash_ignored(self):
        content = " MANTID_BATCH_FILE,foo,bar"
        batch_file_path = BatchCsvParserTest._save_to_csv(content)
        parser = BatchCsvParser()

        output = parser.parse_batch_file(batch_file_path)
        self.assertEqual(0, len(output))

    def test_can_parse_sample_geometries(self):
        batch_file_row = ["sample_sans", "1", "sample_trans", "", "sample_direct_beam", "",
                          "can_sans", "", "can_trans", "", "can_direct_beam", "", "output_as", "", "user_file", "",
                          "sample_thickness", "5", "sample_height", "5", "sample_width", "5"]
        parser = BatchCsvParser()
        parser._parse_csv_row(batch_file_row, 0)

    def test_empty_batch_file_name_throws(self):
        batch_file_path = "not_there.csv"
        parser = BatchCsvParser()
        with self.assertRaises(RuntimeError):
            parser.parse_batch_file(batch_file_path)

    def test_opens_correct_file(self):
        mocked_handle = mock.mock_open()

        expected_file_path = "/foo/bar.csv"
        parser = BatchCsvParser()

        patchable = "__builtin__.open" if six.PY2 else "builtins.open"
        with mock.patch(patchable, mocked_handle):
            parser.save_batch_file(rows=[], file_path=expected_file_path)

        mocked_handle.assert_called_with(expected_file_path, csv_open_type)

    def test_parses_row_to_csv_correctly(self):
        test_row = RowEntries()
        test_row.sample_scatter = "SANS2D00022025"
        test_row.sample_transmission = "SANS2D00022052"
        test_row.sample_direct = "SANS2D00022022"
        test_row.output_name = "another_file"
        test_row.user_file = "a_user_file.txt"
        test_row.sample_thickness = "1.0"
        test_row.sample_height = 5.0
        test_row.sample_width = 5.4

        expected = "sample_sans,SANS2D00022025," \
                   "sample_trans,SANS2D00022052,"\
                   "sample_direct_beam,SANS2D00022022," \
                   "can_sans,," \
                   "can_trans,," \
                   "can_direct_beam,,"\
                   "output_as,another_file," \
                   "user_file,a_user_file.txt," \
                   "sample_thickness,1.0,"\
                   "sample_height,5.0," \
                   "sample_width,5.4"

        mocked_handle = mock.mock_open()
        parser = BatchCsvParser()

        patchable = "__builtin__.open" if six.PY2 else "builtins.open"
        with mock.patch(patchable, mocked_handle):
            parser.save_batch_file(rows=[test_row], file_path='')

        result = mocked_handle()
        args, kwargs = result.write.call_args

        # Strip new lines etc
        self.assertTrue(isinstance(args[0], str))
        written = args[0].replace('\n', '').replace('\r', '')
        self.assertEqual(expected, written)


if __name__ == '__main__':
    unittest.main()
