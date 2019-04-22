# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.
#
#
from __future__ import (absolute_import, division, print_function)
from .model import SliceViewerModel
from .view import SliceViewerView


class SliceViewer(object):
    def __init__(self, ws, parent=None):
        self.model = SliceViewerModel(ws)

        self.view = SliceViewerView(self.model.get_dimensions_info(), parent)
        self.view.dimensionsChanged.connect(self.new_plot)
        self.view.valueChanged.connect(self.update_plot_data)

        self.new_plot()

    def new_plot(self):
        self.view.plot(self.model.get_ws(), slicepoint=self.view.dimensions.get_slicepoint())

    def update_plot_data(self):
        self.view.update_plot_data(self.model.get_data(self.view.dimensions.get_slicepoint()))
