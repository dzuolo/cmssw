import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *
from PhysicsTools.NanoAOD.nano_eras_cff import *


##################### Tables for final output and docs ##########################
metTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("slimmedMETs"),
    name = cms.string("MET"),
    doc = cms.string("slimmedMET, type-1 corrected PF MET"),
    singleton = cms.bool(True),  # there's always exactly one MET per event
    extension = cms.bool(False), # this is the main table for the MET
    variables = cms.PSet(PTVars,
       sumEt = Var("sumEt()", float, doc="scalar sum of Et",precision=10),
       covXX = Var("getSignificanceMatrix().At(0,0)",float,doc="xx element of met covariance matrix", precision=8),
       covXY = Var("getSignificanceMatrix().At(0,1)",float,doc="xy element of met covariance matrix", precision=8),
       covYY = Var("getSignificanceMatrix().At(1,1)",float,doc="yy element of met covariance matrix", precision=8),
       significance = Var("metSignificance()", float, doc="MET significance",precision=10),
       sumPtUnclustered = Var("metSumPtUnclustered()", float, doc="sumPt used for MET significance",precision=10),
       MetUnclustEnUpDeltaX = Var("shiftedPx('UnclusteredEnUp')-px()", float, doc="Delta (METx_mod-METx) Unclustered Energy Up",precision=10),
       MetUnclustEnUpDeltaY = Var("shiftedPy('UnclusteredEnUp')-py()", float, doc="Delta (METy_mod-METy) Unclustered Energy Up",precision=10),

    ),
)


rawMetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = metTable.src,
    name = cms.string("RawMET"),
    doc = cms.string("raw PF MET"),
    singleton = cms.bool(True),  # there's always exactly one MET per event
    extension = cms.bool(False), # this is the main table for the MET
    variables = cms.PSet(#NOTA BENE: we don't copy PTVars here!
       pt  = Var("uncorPt",  float, doc="pt", precision=10),
       phi = Var("uncorPhi", float, doc="phi", precision=10),
       sumEt = Var("uncorSumEt", float, doc="scalar sum of Et", precision=10),
    ),
)


caloMetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = metTable.src,
    name = cms.string("CaloMET"),
    doc = cms.string("Offline CaloMET (muon corrected)"),
    singleton = cms.bool(True),  # there's always exactly one MET per event
    extension = cms.bool(False), # this is the main table for the MET
    variables = cms.PSet(#NOTA BENE: we don't copy PTVars here!
       pt  = Var("caloMETPt",  float, doc="pt", precision=10),
       phi = Var("caloMETPhi", float, doc="phi", precision=10),
       sumEt = Var("caloMETSumEt", float, doc="scalar sum of Et", precision=10),
    ),
)

puppiMetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("slimmedMETsPuppi"),
    name = cms.string("PuppiMET"),
    doc = cms.string("PUPPI  MET"),
    singleton = cms.bool(True),  # there's always exactly one MET per event
    extension = cms.bool(False), # this is the main table for the MET
    variables = cms.PSet(PTVars,
       sumEt = Var("sumEt()", float, doc="scalar sum of Et",precision=10),
       ptJERUp = Var("shiftedPt('JetResUp')", float, doc="JER up pt",precision=10),
       ptJERDown = Var("shiftedPt('JetResDown')", float, doc="JER down pt",precision=10),
       phiJERUp = Var("shiftedPhi('JetResUp')", float, doc="JER up phi",precision=10),
       phiJERDown = Var("shiftedPhi('JetResDown')", float, doc="JER down phi",precision=10),
       ptJESUp = Var("shiftedPt('JetEnUp')", float, doc="JES up pt",precision=10),
       ptJESDown = Var("shiftedPt('JetEnDown')", float, doc="JES down pt",precision=10),
       phiJESUp = Var("shiftedPhi('JetEnUp')", float, doc="JES up phi",precision=10),
       phiJESDown = Var("shiftedPhi('JetEnDown')", float, doc="JES down phi",precision=10),
       ptUnclusteredUp = Var("shiftedPt('UnclusteredEnUp')", float, doc="Unclustered up pt",precision=10),
       ptUnclusteredDown = Var("shiftedPt('UnclusteredEnDown')", float, doc="Unclustered down pt",precision=10),
       phiUnclusteredUp = Var("shiftedPhi('UnclusteredEnUp')", float, doc="Unclustered up phi",precision=10),
       phiUnclusteredDown = Var("shiftedPhi('UnclusteredEnDown')", float, doc="Unclustered down phi",precision=10),
    ),
)

rawPuppiMetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = puppiMetTable.src,
    name = cms.string("RawPuppiMET"),
    doc = cms.string("raw Puppi MET"),
    singleton = cms.bool(True),  # there's always exactly one MET per event
    extension = cms.bool(False), # this is the main table for the MET
    variables = cms.PSet(#NOTA BENE: we don't copy PTVars here!
       pt  = Var("uncorPt",  float, doc="pt", precision=10),
       phi = Var("uncorPhi", float, doc="phi", precision=10),
       sumEt = Var("uncorSumEt", float, doc="scalar sum of Et", precision=10),
    ),)


tkMetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = metTable.src,
    name = cms.string("TkMET"),
    doc = cms.string("Track MET computed with tracks from PV0 ( pvAssociationQuality()>=4 ) "),
    singleton = cms.bool(True),  # there's always exactly one MET per event
    extension = cms.bool(False), # this is the main table for the TkMET
    variables = cms.PSet(#NOTA BENE: we don't copy PTVars here!
       pt = Var("corPt('RawTrk')", float, doc="raw track MET pt",precision=10),
       phi = Var("corPhi('RawTrk')", float, doc="raw track MET phi",precision=10),
       sumEt = Var("corSumEt('RawTrk')", float, doc="raw track scalar sum of Et",precision=10),
    ),
)

chsMetTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = metTable.src,
    name = cms.string("ChsMET"),
    doc = cms.string("PF MET computed with CHS PF candidates"),
    singleton = cms.bool(True),  # there's always exactly one MET per event
    extension = cms.bool(False), # this is the main table for the TkMET
    variables = cms.PSet(#NOTA BENE: we don't copy PTVars here!
       pt = Var("corPt('RawChs')", float, doc="raw chs PF MET pt",precision=10),
       phi = Var("corPhi('RawChs')", float, doc="raw chs PF MET phi",precision=10),
       sumEt = Var("corSumEt('RawChs')", float, doc="raw chs PF scalar sum of Et",precision=10),
    ),
)

deepMetResolutionTuneTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    # current deepMets are saved in slimmedMETs in MiniAOD,
    # in the same way as chsMet/TkMET
    src = metTable.src,
    name = cms.string("DeepMETResolutionTune"),
    doc = cms.string("Deep MET trained with resolution tune"),
    singleton = cms.bool(True), # there's always exactly one MET per event
    extension = cms.bool(False), # this is the main table for the MET
    variables = cms.PSet(#NOTA BENE: we don't copy PTVars here!
        pt = Var("corPt('RawDeepResolutionTune')", float, doc="DeepMET ResolutionTune pt",precision=-1),
        phi = Var("corPhi('RawDeepResolutionTune')", float, doc="DeepmET ResolutionTune phi",precision=12),
    ),
)

deepMetResponseTuneTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = metTable.src,
    name = cms.string("DeepMETResponseTune"),
    doc = cms.string("Deep MET trained with extra response tune"),
    singleton = cms.bool(True), # there's always exactly one MET per event
    extension = cms.bool(False), # this is the main table for the MET
    variables = cms.PSet(#NOTA BENE: we don't copy PTVars here!
        pt = Var("corPt('RawDeepResponseTune')", float, doc="DeepMET ResponseTune pt",precision=-1),
        phi = Var("corPhi('RawDeepResponseTune')", float, doc="DeepMET ResponseTune phi",precision=12),
    ),
)

metFixEE2017Table = metTable.clone()
metFixEE2017Table.src = cms.InputTag("slimmedMETsFixEE2017")
metFixEE2017Table.name = cms.string("METFixEE2017")
metFixEE2017Table.doc = cms.string("Type-1 corrected PF MET, with fixEE2017 definition")


metMCTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = metTable.src,
    name = cms.string("GenMET"),
    doc = cms.string("Gen MET"),
    singleton = cms.bool(True),  
    extension = cms.bool(False),
    variables = cms.PSet(
       pt  = Var("genMET.pt",  float, doc="pt", precision=10),
       phi = Var("genMET.phi", float, doc="phi", precision=10),
    ),
)


metTablesTask = cms.Task( metTable, rawMetTable, caloMetTable, puppiMetTable, rawPuppiMetTable, tkMetTable, chsMetTable)
deepMetTablesTask = cms.Task( deepMetResolutionTuneTable, deepMetResponseTuneTable )
_withFixEE2017_task = cms.Task(metTablesTask.copy(), metFixEE2017Table)
for modifier in run2_nanoAOD_94XMiniAODv1, run2_nanoAOD_94XMiniAODv2:
    modifier.toReplaceWith(metTablesTask,_withFixEE2017_task) # only in old miniAOD, the new ones will come from the UL rereco
metMCTask = cms.Task( metMCTable )
