For Protein-ligand design, the atom-parameter file and residue-topology file must be generated before performing protein design.

Step 1: ../../../UniDesign.exe --command=GenLigParamAndTopo --mol2=lig_charge.mol2 --lig_atomparam=1r091_lig_param.prm --lig_topology=1r091_lig_topo.inp
If the options --lig_atomparam and --lig_topology are not specified, ligand atomic parameters and topology will be written into file LIGAND_PARAM.prm and LIGAND_TOPO.inp, respectively, under the current directory where you are running the UniDesign program.

Step 2: ../../../UniDesign --command=ProteinDesign --protlig --pdb=rec_native.pdb --mol2=lig_charge.mol2 --lig_atomparam=1r091_lig_param.prm --lig_topology=1r091_lig_topo.inp
If the options --lig_atomparam and --lig_topology are not specified, the UniDesign program will look for LIGAND_PARAM.prm and LIGAND_TOPO.inp in the current directory where you are running the UniDesign program.

If the first step has not been executed before running Step 2, the UniDesign program will return an error message and exit. 


