from mantid.simpleapi import *
from IndirectCommon import *
from IndirectImport import import_mantidplot
mp = import_mantidplot()
from mantid import config, logger
import inelastic_indirect_reducer
import sys, os.path, numpy as np

def loadData(rawfiles, outWS='RawFile', Sum=False, SpecMin=-1, SpecMax=-1,
        Suffix=''):
    workspaces = []
    for file in rawfiles:
        ( dir, filename ) = os.path.split(file)
        ( name, ext ) = os.path.splitext(filename)
        try:
            if ( SpecMin == -1 ) and ( SpecMax == -1 ):
                Load(Filename=file, OutputWorkspace=name+Suffix, LoadLogFiles=False)
            else:
                Load(Filename=file, OutputWorkspace=name+Suffix, SpectrumMin=SpecMin,
                    SpectrumMax=SpecMax, LoadLogFiles=False)
            workspaces.append(name+Suffix)
        except ValueError, message:
            logger.notice(message)
            sys.exit(message)
    if Sum and ( len(workspaces) > 1 ):
        MergeRuns(InputWorkspaces=','.join(workspaces), OutputWorkspace=outWS+Suffix)
        factor = 1.0 / len(workspaces)
        Scale(InputWorkspace=outWS+Suffix, OutputWorkspace=outWS+Suffix, Factor=factor)
        for ws in workspaces:
            DeleteWorkspace(ws)
        return [outWS+Suffix]
    else:
        return workspaces

def resolution(files, iconOpt, rebinParam, bground,
        instrument, analyser, reflection,
        Res=True, factor=None, Plot=False, Verbose=False, Save=False):
    reducer = inelastic_indirect_reducer.IndirectReducer()
    reducer.set_instrument_name(instrument)
    reducer.set_detector_range(iconOpt['first']-1,iconOpt['last']-1)
    for file in files:
        reducer.append_data_file(file)
    parfile = instrument +"_"+ analyser +"_"+ reflection +"_Parameters.xml"
    reducer.set_parameter_file(parfile)
    reducer.set_grouping_policy('All')
    reducer.set_sum_files(True)

    try:
        reducer.reduce()
    except Exception, e:
        logger.error(str(e))
        return

    iconWS = reducer.get_result_workspaces()[0]

    if factor != None:
        Scale(InputWorkspace=iconWS, OutputWorkspace=iconWS, Factor = factor)

    if Res:
        name = getWSprefix(iconWS) + 'res'
        CalculateFlatBackground(InputWorkspace=iconWS, OutputWorkspace=name, StartX=bground[0], EndX=bground[1],
            Mode='Mean', OutputMode='Subtract Background')
        Rebin(InputWorkspace=name, OutputWorkspace=name, Params=rebinParam)

        if Save:
            if Verbose:
                logger.notice("Resolution file saved to default save directory.")
            SaveNexusProcessed(InputWorkspace=name, Filename=name+'.nxs')

        if Plot:
            graph = mp.plotSpectrum(name, 0)
        return name
    else:
        if Plot:
            graph = mp.plotSpectrum(iconWS, 0)
        return iconWS

##############################################################################
# Slice Functions
##############################################################################

def sliceReadRawFile(fname, Verbose):

    if Verbose:
        logger.notice('Reading file :'+fname)

    #Load the raw file
    (dir, filename) = os.path.split(fname)
    (root, ext) = os.path.splitext(filename)

    Load(Filename=fname, OutputWorkspace=root, LoadLogFiles=False)

    return root

# returns the number of monitors
# and if they're at the start or end of the file
def countMonitors(rawFile):
    rawFile = mtd[rawFile]
    nhist = rawFile.getNumberHistograms()
    detector = rawFile.getDetector(0)
    monCount = 1

    if detector.isMonitor():
        #monitors are at the start
        for i in range(1,nhist):
            detector = rawFile.getDetector(i)

            if detector.isMonitor():
                monCount += 1
            else:
                break

        return monCount, True
    else:
        #monitors are at the end
        detector = rawFile.getDetector(nhist)

        if not detector.isMonitor():
            #if it's not, we don't have any monitors!
            return 0, True

        for i in range(nhist,0,-1):
            detector = rawFile.getDetector(i)

            if detector.isMonitor():
                monCount += 1
            else:
                break

        return monCount, False

# Run the calibration file with the raw file workspace
def sliceProcessCalib(rawFile, calibWsName, spec):
    calibSpecMin, calibSpecMax = spec

    if calibSpecMax-calibSpecMin > mtd[calibWsName].getNumberHistograms():
        raise IndexError("Number of spectra used is greater than the number of spectra in the calibration file.")

    #offset cropping range to account for monitors
    (monCount, atStart) = countMonitors(rawFile)

    if atStart:
        calibSpecMin -= monCount+1
        calibSpecMax -= monCount+1

    #Crop the calibration workspace, excluding the monitors
    CropWorkspace(InputWorkspace=calibWsName, OutputWorkspace=calibWsName,
        StartWorkspaceIndex=calibSpecMin, EndWorkspaceIndex=calibSpecMax)

def sliceProcessRawFile(rawFile, calibWsName, useCalib, xRange, useTwoRanges, spec, suffix, Verbose):

    #Crop the raw file to use the desired number of spectra
    #less one because CropWorkspace is zero based
    CropWorkspace(InputWorkspace=rawFile, OutputWorkspace=rawFile,
        StartWorkspaceIndex=spec[0]-1, EndWorkspaceIndex=spec[1]-1)

    nhist,ntc = CheckHistZero(rawFile)

    #use calibration file if desired
    if useCalib:
        Divide(LHSWorkspace=rawFile, RHSWorkspace=calibWsName, OutputWorkspace=rawFile)

    #construct output workspace name
    run = mtd[rawFile].getRun().getLogData("run_number").value
    sfile = rawFile[:3].lower() + run + '_' + suffix + '_slice'

    if not useTwoRanges:
        Integration(InputWorkspace=rawFile, OutputWorkspace=sfile, RangeLower=xRange[0], RangeUpper=xRange[1],
            StartWorkspaceIndex=0, EndWorkspaceIndex=nhist-1)
    else:
        CalculateFlatBackground(InputWorkspace=rawFile, OutputWorkspace=sfile, StartX=xRange[2], EndX=xRange[3],
                Mode='Mean')
        Integration(InputWorkspace=sfile, OutputWorkspace=sfile, RangeLower=xRange[0], RangeUpper=xRange[1],
            StartWorkspaceIndex=0, EndWorkspaceIndex=nhist-1)

    return sfile

def slice(inputfiles, calib, xRange, spec, suffix, Save=False, Verbose=False, Plot=False):

    StartTime('Slice')

    CheckXrange(xRange,'Time')

    workdir = config['defaultsave.directory']

    outWSlist = []
    useTwoRanges = (len(xRange) != 2)
    useCalib = (calib != '')
    calibWsName = '__calibration'

    #load the calibration file
    if useCalib:
        Load(Filename=calib, OutputWorkspace=calibWsName)
        if Verbose:
            logger.notice('Using Calibration file: %s' % calib)

    for index, file in enumerate(inputfiles):
        rawFile = sliceReadRawFile(file, Verbose)

        #only need to process the calib file once
        if(index == 0 and useCalib):
            sliceProcessCalib(rawFile, calibWsName, spec)

        sfile = sliceProcessRawFile(rawFile, calibWsName, useCalib, xRange, useTwoRanges, spec, suffix, Verbose)
        Transpose(InputWorkspace=sfile, OutputWorkspace=sfile)
        unit = mtd[sfile].getAxis(0).setUnit("Label")
        unit.setLabel("Spectrum Number", "")

        outWSlist.append(sfile)
        DeleteWorkspace(rawFile)

        if Save:
            # path name for nxs file
            o_path = os.path.join(workdir, sfile+'.nxs')
            SaveNexusProcessed(InputWorkspace=sfile, Filename=o_path)

            if Verbose:
                logger.notice('Output file :'+o_path)

    if useCalib:
        DeleteWorkspace(Workspace=calibWsName)

    if Plot:
        try:
            graph = mp.plotSpectrum(sfile, 0)
        except RuntimeError, e:
            #User clicked cancel on plot so don't do anything
            pass

    EndTime('Slice')

def getInstrumentDetails(instrument):
    instr_name = '__empty_' + instrument
    if mtd.doesExist(instr_name):
        workspace = mtd[instr_name]
    else:
        idf_dir = config['instrumentDefinition.directory']
        idf = idf_dir + instrument + '_Definition.xml'
        LoadEmptyInstrument(Filename=idf, OutputWorkspace=instr_name)
        workspace = mtd[instr_name]
    instrument = workspace.getInstrument()
    ana_list_param = instrument.getStringParameter('analysers')
    if len(ana_list_param) != 1:
        return ""
    ana_list_split = ana_list_param[0].split(',')
    reflections = []
    result = ''
    for analyser in ana_list_split:
        list = []
        name = 'refl-' + analyser
        list.append( analyser )
        try:
            item = instrument.getStringParameter(name)[0]
        except IndexError:
            item = ''
        refl = item.split(',')
        list.append( refl )
        reflections.append(list)
    for i in range(0, len(reflections)):
        message = reflections[i][0] + '-'
        for j in range(0,len(reflections[i][1])):
            message += str(reflections[i][1][j])
            if j < ( len(reflections[i][1]) -1 ):
                message += ','
        result += message
        if ( i < ( len(reflections) - 1) ):
            result += '\n'
    return result

def getReflectionDetails(inst, analyser, refl):
    idf_dir = config['instrumentDefinition.directory']
    ws = '__empty_' + inst
    if not mtd.doesExist(ws):
        idf_file = inst + '_Definition.xml'
        idf = os.path.join(idf_dir, idf_file)
        LoadEmptyInstrument(Filename=idf, OutputWorkspace=ws)
    ipf_file = inst + '_' + analyser + '_' + refl + '_Parameters.xml'
    ipf = os.path.join(idf_dir, ipf_file)
    LoadParameterFile(Workspace=ws, Filename=ipf)
    inst = mtd[ws].getInstrument()
    result = ''
    try:
        result += str( inst.getStringParameter('analysis-type')[0] ) + '\n'
        result += str( int(inst.getNumberParameter('spectra-min')[0]) ) + '\n'
        result += str( int(inst.getNumberParameter('spectra-max')[0]) ) + '\n'
        result += str( inst.getNumberParameter('efixed-val')[0] ) + '\n'
        result += str( int(inst.getNumberParameter('peak-start')[0]) ) + '\n'
        result += str( int(inst.getNumberParameter('peak-end')[0]) ) + '\n'
        result += str( int(inst.getNumberParameter('back-start')[0]) ) + '\n'
        result += str( int(inst.getNumberParameter('back-end')[0]) ) + '\n'
        result += inst.getStringParameter('rebin-default')[0]
    except IndexError:
        pass
    return result
