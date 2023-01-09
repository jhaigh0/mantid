# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
# pylint: disable=no-init,invalid-name
from mantid.api import (
    PythonAlgorithm,
    AlgorithmFactory,
    ITableWorkspaceProperty,
    WorkspaceFactory,
    FileProperty,
    FileAction,
    MatrixWorkspaceProperty,
)
from mantid.kernel import Direction

_OUTPUTLEVEL = "NOOUTPUT"


class LoadFullprofFile(PythonAlgorithm):
    """Create the input TableWorkspaces for LeBail Fitting"""

    _tableWS = None
    _dataWS = None

    def category(self):
        """ """
        return "Diffraction\\DataHandling"

    def seeAlso(self):
        return ["LoadFullprofResolution"]

    def name(self):
        """ """
        return "LoadFullprofFile"

    def summary(self):
        """Return summary"""
        return "Load file generated by Fullprof."

    def PyInit(self):
        """Declare properties"""
        self.declareProperty(
            FileProperty("Filename", "", FileAction.Load, [".hkl", ".prf", ".dat"]),
            "Name of [http://www.ill.eu/sites/fullprof/ Fullprof] .hkl or .prf file.",
        )

        # self.declareProperty("Bank", 1, "Bank ID for output if there are more than one bank in .irf file.")

        self.declareProperty(
            ITableWorkspaceProperty("PeakParameterWorkspace", "", Direction.Output),
            "Name of table workspace containing peak parameters from .hkl file.",
        )

        self.declareProperty(
            MatrixWorkspaceProperty("OutputWorkspace", "", Direction.Output),
            "Name of data workspace containing the diffraction pattern in .prf file. ",
        )

        return

    def PyExec(self):
        """Main Execution Body"""
        # 1. Properties
        fpfilename = self.getPropertyValue("Filename")

        # 2. Import
        if fpfilename.lower().endswith(".hkl") is True:
            # (.hkl) file
            self._tableWS = self._loadFPHKLFile(fpfilename)
            self._dataWS = self._makeEmptyDataWorkspace()
        elif fpfilename.lower().endswith(".prf") is True:
            # (.prf) file
            self._tableWS, self._dataWS = self._loadFullprofPrfFile(fpfilename)
        elif fpfilename.lower().endswith(".dat") is True:
            # (.dat) file: Fullprof data file
            self._tableWS, self._dataWS = self._loadFullprofDataFile(fpfilename)
        else:
            raise NotImplementedError("File %s is neither .hkl nor .prf.  It is not supported." % (fpfilename))

        # 3. Export
        self.setProperty("PeakParameterWorkspace", self._tableWS)
        self.setProperty("OutputWorkspace", self._dataWS)

        return

    def _loadFPHKLFile(self, filename):
        """Load Fullprof .hkl file to a TableWorkspace"""
        # 1. Import to a dictionary
        hkldict = self._importFullprofHKLFile(filename)

        # 2. Create workspace from dictionary
        peakws = self._createReflectionWorkspace(hkldict)

        return peakws

    def _importFullprofHKLFile(self, hklfilename):
        """Import Fullprof's .hkl file"""
        import math

        # 1. Import file
        try:
            hklfile = open(hklfilename, "r")
            lines = hklfile.readlines()
            hklfile.close()
        except IOError:
            raise IOError("Error to open/read Fullprof .hkl file %s" % (hklfilename))

        # 2. Parse
        hkldict = {}
        for line in lines:
            # a) Clean & split
            line = line.strip()
            if len(line) == 0:
                continue
            terms = line.split()

            # b) parse
            if not terms[0].isdigit():
                # Comment line
                continue

            h = int(terms[0])
            k = int(terms[1])
            l = int(terms[2])
            if len(terms) >= 9:
                dsp = float(terms[3])
                tof = float(terms[4])
                alpha = float(terms[5])
                beta = float(terms[6])
                sigma2 = float(terms[7])
                gamma2 = float(terms[8])
            else:
                dsp = 0.0
                tof = 0.0
                alpha = 0.0
                beta = 0.0
                sigma2 = 0.0
                gamma2 = 0.0
                fwhm = 1.0

            if len(terms) >= 13:
                fwhm = float(terms[12])
            elif len(terms) >= 9:
                fwhm = math.sqrt(sigma2) * 2.0

            dkey = (h, k, l)

            if dkey in hkldict:
                if _OUTPUTLEVEL == "INFORMATION":
                    self.warning("Warning! Duplicate HKL %d, %d, %d" % (h, k, l))
                continue

            if fwhm < 1.0e-5:
                # Peak width is too small/annihilated peak
                if _OUTPUTLEVEL == "INFORMATION":
                    self.log.information("Peak (%d, %d, %d) has an unreasonable small FWHM.  Peak does not exist. " % (h, k, l))
                continue

            hkldict[dkey] = {}
            hkldict[dkey]["dsp"] = dsp
            hkldict[dkey]["tof"] = tof
            hkldict[dkey]["alpha"] = alpha
            hkldict[dkey]["beta"] = beta
            hkldict[dkey]["sigma2"] = sigma2
            hkldict[dkey]["gamma2"] = gamma2
            hkldict[dkey]["FWHM"] = fwhm
        # ENDFOR: line

        self.log().information("Import Fullprof reflection file %s successfully. " % (hklfilename))

        return hkldict

    def _createReflectionWorkspace(self, hkldict):
        """Create TableWorkspace containing reflections and etc."""
        # 1. Set up columns
        tablews = WorkspaceFactory.createTable()

        tablews.addColumn("int", "H")
        tablews.addColumn("int", "K")
        tablews.addColumn("int", "L")
        tablews.addColumn("double", "Alpha")
        tablews.addColumn("double", "Beta")
        tablews.addColumn("double", "Sigma2")
        tablews.addColumn("double", "Gamma")
        tablews.addColumn("double", "FWHM")
        tablews.addColumn("double", "PeakHeight")

        # 2. Add rows
        for hkl in sorted(hkldict.keys()):
            pardict = hkldict[hkl]
            tablews.addRow(
                [hkl[0], hkl[1], hkl[2], pardict["alpha"], pardict["beta"], pardict["sigma2"], pardict["gamma2"], pardict["FWHM"], 1.0]
            )
        # ENDFOR

        return tablews

    def _loadFullprofPrfFile(self, prffilename):
        """Load Fullprof .prf file"""
        # 1. Parse the file to dictionary
        infodict, data = self._parseFullprofPrfFile(prffilename)

        # 2. Export information to table file
        tablews = WorkspaceFactory.createTable()
        tablews.addColumn("str", "Name")
        tablews.addColumn("double", "Value")
        for parname in infodict:
            parvalue = infodict[parname]
            tablews.addRow([parname, parvalue])

        # 3. Export the data workspace
        datasize = len(data)
        print("Data Size = ", datasize)
        dataws = WorkspaceFactory.create("Workspace2D", 4, datasize, datasize)
        for i in range(datasize):
            for j in range(4):
                dataws.dataX(j)[i] = data[i][0]
                dataws.dataY(j)[i] = data[i][j + 1]
                dataws.dataE(j)[i] = 1.0

        return (tablews, dataws)

    # pylint: disable=too-many-locals, too-many-branches
    def _parseFullprofPrfFile(self, filename):
        """Parse Fullprof .prf file to a information dictionary and a data set (list of list)"""
        import re

        # Import .prf file
        try:
            pfile = open(filename, "r")
        except IOError:
            raise NotImplementedError("Unable to open .prf file %s" % (filename))

        rawlines = pfile.readlines()
        lines = []
        for l in rawlines:
            line = l.strip()
            if len(line) > 0:
                lines.append(line)

        infodict = {}
        dataset = []

        # Parse information lines
        # Line 0: header
        infoline = lines[0]
        if infoline.count("CELL:") == 1:
            terms = infoline.split("CELL:")[1].split()
            a = float(terms[0])
            b = float(terms[1])
            c = float(terms[2])
            alpha = float(terms[3])
            beta = float(terms[4])
            gamma = float(terms[5])
            infodict["A"] = float(a)
            infodict["B"] = float(b)
            infodict["C"] = float(c)
            infodict["Alpha"] = float(alpha)
            infodict["Beta"] = float(beta)
            infodict["Gamma"] = float(gamma)
        # if infoline.count("SPGR:") == 1:
        #    terms = infoline.split("SPGR:")
        #    spacegroup = terms[1].strip()
        #    infodict["SpaceGroup"] = spacegroup

        # Find data line header
        firstline = -1
        for i in range(1, len(lines)):
            if lines[i].count("Yobs-Ycal") > 0:
                firstline = i + 1
                # dataheader = lines[i].strip()
                break

        if firstline < 0:
            raise NotImplementedError("File format is incorrect. Unable to locate data title line")

        # Parse header line: T.O.F. Yobs    Ycal    Yobs-Ycal       Backg   Bragg ...
        #     to determine how the data line look alike (==5 or >= 5)
        # headerterms = dataheader.split()
        # dataperline = 5
        # TOF., ... h k l ...
        # reflectionperline = len(headerterms)-5+3

        # Parse data
        count = 0
        for i in range(firstline, len(lines)):
            line = lines[i].strip()
            if len(line) == 0:  # empty line
                continue

            if line.count(")") == 0 and line.count("(") == 0:
                # A regular data line
                terms = line.split()
                if len(terms) != 5:
                    self.log().warning("Pure data line %d (%s) has irregular number of data points" % (i, line))
                    continue

                x = float(terms[0])
                yobs = float(terms[1])
                ycal = float(terms[2])
                ydif = float(terms[3])
                ybak = float(terms[4])

                dataset.append([x, yobs, ycal, ydif, ybak])
                count += 1

            elif line.count(")") == 1 and line.count("(") == 1:
                # A line can be either pure reflection line or a combined data/reflection line
                # remove '(' and ')'
                newline = re.sub("[()]", " ", line)

                terms = newline.strip().split()
                if len(terms) < 9:
                    # Pure reflection line
                    tofh = float(terms[0])
                    hklstr = line.split(")")[1].split(")")[0].strip()
                    infodict[hklstr] = tofh

                else:
                    # Mixed line: least number of items: data(5) + TOF+hkl = 9

                    x = float(terms[0])
                    yobs = float(terms[1])
                    ycal = float(terms[2])
                    ydif = float(terms[3])
                    ybak = float(terms[4])

                    dataset.append([x, yobs, ycal, ydif, ybak])
                    count += 1

                    raise NotImplementedError("Need a sample line of this use case.")
                    # hklstr = line.split(")")[1].split(")")[0].strip()
                    # infodict[hklstr] = tofh
                # ENDIFELSE (terms)
            else:
                self.log().warning("%d-th line (%s) is not well-defined." % (i, line))

            # ENDIF-ELIF-ELSE (line.count)
        # ENDFOR

        print("Data set counter = ", count)

        return (infodict, dataset)

    def _makeEmptyDataWorkspace(self):
        """Make an empty data workspace (Workspace2D)"""
        dataws = WorkspaceFactory.create("Workspace2D", 1, 1, 1)

        return dataws

    def _loadFullprofDataFile(self, datafilename):
        """Parse a Fullprof (multiple) column file"""
        # Import file
        datafile = open(datafilename, "r")
        rawlines = datafile.readlines()
        datafile.close()

        # Parse head
        iline = 0
        parseheader = True
        # title = ""
        while iline < len(rawlines) and parseheader is True:
            line = rawlines[iline].strip()
            if len(line) > 0:
                if line.count("BANK") != 0:
                    # line information
                    terms = line.split()
                    if terms[0] != "BANK":
                        raise NotImplementedError("First word must be 'BANK', but not %s" % (terms[0]))
                    # bankid = int(terms[1])
                    numdata = int(terms[2])
                    numlines = int(terms[3])

                    parseheader = False
                # ENDIF
            # ENDIF
            iline += 1
        # ENDWHILE (iline)

        # Data vectors
        vecx = []
        vecy = []
        vece = []

        for i in range(iline, len(rawlines)):
            line = rawlines[i].strip()
            if len(line) == 0:
                continue

            terms = line.split()
            numitems = len(terms)
            if numitems % 3 != 0:
                print("%d-th line '%s' is not a data line" % (i, line))
                continue

            numpts = numitems / 3
            for j in range(numpts):
                x = float(terms[j * 3])
                y = float(terms[j * 3 + 1])
                e = float(terms[j * 3 + 2])

                vecx.append(x)
                vecy.append(y)
                vece.append(e)
            # ENDFOR
        # ENDFOR (i)

        # Check
        self.log().notice("Expected to read %d data points; Exactly read %d data points. " % (numdata * numlines, len(vecx)))

        # Create output workspaces
        tablews = WorkspaceFactory.createTable()

        # Create the data workspace
        datasize = len(vecx)
        dataws = WorkspaceFactory.create("Workspace2D", 1, datasize, datasize)
        for i in range(datasize):
            dataws.dataX(0)[i] = vecx[i]
            dataws.dataY(0)[i] = vecy[i]
            dataws.dataE(0)[i] = vece[i]

        return (tablews, dataws)


# Register algorithm with Mantid
AlgorithmFactory.subscribe(LoadFullprofFile)
