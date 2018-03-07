noinst_LTLIBRARIES += third_party/superLU_MT/libsuperlu.la

third_party_superLU_MT_libsuperlu_la_CPPFLAGS =
third_party_superLU_MT_libsuperlu_la_CPPFLAGS += -D_PTHREAD
third_party_superLU_MT_libsuperlu_la_CPPFLAGS += -DAdd_
third_party_superLU_MT_libsuperlu_la_CPPFLAGS += $(AM_CPPFLAGS)

third_party_superLU_MT_libsuperlu_la_LDFLAGS =
third_party_superLU_MT_libsuperlu_la_LDFLAGS += $(AM_LDFLAGS)

third_party_superLU_MT_libsuperlu_la_LIBADD =
third_party_superLU_MT_libsuperlu_la_LIBADD += $(PTHREAD_CFLAGS)
third_party_superLU_MT_libsuperlu_la_LIBADD += $(PTHREAD_LIBS)
third_party_superLU_MT_libsuperlu_la_LIBADD += -lm

third_party_superLU_MT_libsuperlu_la_SOURCES =
# from third_party/CBLAS
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/dasum.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/daxpy.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/dcopy.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/ddot.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/dgemv.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/dger.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/dnrm2.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/drot.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/dscal.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/dsymv.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/dsyr2.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/dtrsv.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/f2c.h
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/idamax.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/slu_Cnames.h
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/CBLAS/superlu_f2c.h
# from third_party/superLU_MT
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/await.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/colamd.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/colamd.h
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dclock.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dgscon.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dgsequ.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dgsrfs.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dgstrs.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dlacon.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dlamch.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dlangs.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dlaqgs.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dmatgen.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dmyblas2.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dpivotgrowth.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dreadhb.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dsp_blas2.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/dsp_blas3.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/get_perm_c.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/heap_relax_snode.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/lsame.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/mmd.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgssv.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgssvx.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_bmod1D.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_bmod1D_mv2.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_bmod2D.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_bmod2D_mv2.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_column_bmod.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_column_dfs.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_copy_to_ucol.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_factor_snode.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_init.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_panel_bmod.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_panel_dfs.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_pivotL.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_snode_bmod.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_snode_dfs.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_thread.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_thread_finalize.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdgstrf_thread_init.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdmemory.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdsp_defs.h
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pdutil.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pmemory.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pxgstrf_finalize.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pxgstrf_mark_busy_descends.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pxgstrf_pruneL.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pxgstrf_relax_snode.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pxgstrf_scheduler.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pxgstrf_super_bnd_dfs.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pxgstrf_synch.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/pxgstrf_synch.h
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/qrnzcnt.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/slu_mt_Cnames.h
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/slu_mt_machines.h
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/slu_mt_util.h
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/sp_coletree.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/sp_colorder.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/cholnzcnt.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/sp_ienv.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/superlu_timer.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/supermatrix.h
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/util.c
third_party_superLU_MT_libsuperlu_la_SOURCES += third_party/superLU_MT/xerbla.c

