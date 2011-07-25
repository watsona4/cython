////////// MemviewSliceStruct.proto //////////


/* memoryview slice struct */

typedef struct {
  struct {{memview_struct_name}} *memview;
  /* For convenience and faster access */
  char *data;
  Py_ssize_t shape[{{max_dims}}];
  Py_ssize_t strides[{{max_dims}}];
  Py_ssize_t suboffsets[{{max_dims}}];
} {{memviewslice_name}};

/////////////// ObjectToMemviewSlice.proto ///////////////
{{# __Pyx_PyObject_to_MemoryviewSlice_<count> }}
static CYTHON_INLINE {{memviewslice_name}} {{funcname}}(PyObject *);

////////// MemviewSliceInit.proto //////////

#define __Pyx_BUF_MAX_NDIMS %(BUF_MAX_NDIMS)d

#define __Pyx_MEMVIEW_DIRECT   1
#define __Pyx_MEMVIEW_PTR      2
#define __Pyx_MEMVIEW_FULL     4
#define __Pyx_MEMVIEW_CONTIG   8
#define __Pyx_MEMVIEW_STRIDED  16
#define __Pyx_MEMVIEW_FOLLOW   32

#define __Pyx_IS_C_CONTIG 1
#define __Pyx_IS_F_CONTIG 2


/* #define __PYX_MEMSLICE_GETDATA(SLICE) ((char *) SLICE->memview->view->buf) */

static int __Pyx_ValidateAndInit_memviewslice(struct __pyx_memoryview_obj *memview,
                                int *axes_specs, int c_or_f_flag,  int ndim, __Pyx_TypeInfo *dtype,
                                __Pyx_BufFmt_StackElem stack[], __Pyx_memviewslice *memviewslice);

static int __Pyx_init_memviewslice(
                struct __pyx_memoryview_obj *memview,
                int ndim,
                __Pyx_memviewslice *memviewslice);

#define __PYX_INC_MEMVIEW(slice, have_gil) __Pyx_INC_MEMVIEW(slice, have_gil, __LINE__)
#define __PYX_XDEC_MEMVIEW(slice, have_gil) __Pyx_XDEC_MEMVIEW(slice, have_gil, __LINE__)
static CYTHON_INLINE void __Pyx_INC_MEMVIEW({{memviewslice_name}} *, int, int);
static CYTHON_INLINE void __Pyx_XDEC_MEMVIEW({{memviewslice_name}} *, int, int);

/////////////// MemviewSliceIndex.proto ///////////////

static CYTHON_INLINE char *__pyx_memviewslice_index_full(char *bufp, Py_ssize_t idx, Py_ssize_t stride, Py_ssize_t suboffset);
static CYTHON_INLINE char *__pyx_memviewslice_index_full_contig(char *bufp, Py_ssize_t suboffset);


/////////////// ObjectToMemviewSlice ///////////////

{{#__Pyx_PyObject_to_MemoryviewSlice_<count>}}

static CYTHON_INLINE {{memviewslice_name}} {{funcname}}(PyObject *obj) {
    {{memviewslice_name}} result;
    result.memview = NULL;
    result.data = NULL;
    struct __pyx_memoryview_obj *memview =  \
        (struct __pyx_memoryview_obj *) __pyx_memoryview_new(obj, {{buf_flag}});
    __Pyx_BufFmt_StackElem stack[{{struct_nesting_depth}}];
    int axes_specs[] = { {{axes_specs}} };
    int retcode;

    if (unlikely(!memview))
        goto __pyx_fail;

    retcode = __Pyx_ValidateAndInit_memviewslice(memview, axes_specs,
            {{c_or_f_flag}}, {{ndim}}, &{{dtype_typeinfo}}, stack, &result);

    if (unlikely(retcode == -1))
        goto __pyx_fail;

    memview->acquisition_count = 1;
    return result;
__pyx_fail:
    Py_XDECREF(memview);
    result.memview = NULL;
    result.data = NULL;
    return result;
}

////////// MemviewSliceInit //////////

static int __Pyx_ValidateAndInit_memviewslice(
                struct __pyx_memoryview_obj *memview,
                int *axes_specs,
                int c_or_f_flag,
                int ndim,
                __Pyx_TypeInfo *dtype,
                __Pyx_BufFmt_StackElem stack[],
                __Pyx_memviewslice *memviewslice) {

    __Pyx_RefNannyDeclarations
    __Pyx_RefNannySetupContext("ValidateAndInit_memviewslice");
    Py_buffer *buf = &memview->view;
    int stride, i, spec = 0, retval = -1;

    if (!buf) goto fail;

    if(memviewslice->data || memviewslice->memview) {
        PyErr_SetString(PyExc_ValueError,
            "memoryviewslice struct must be initialized to NULL.");
        goto fail;
    }

    if (buf->ndim != ndim) {
        PyErr_Format(PyExc_ValueError,
                "Buffer has wrong number of dimensions (expected %d, got %d)",
                ndim, buf->ndim);
        goto fail;
    }

    __Pyx_BufFmt_Context ctx;
    __Pyx_BufFmt_Init(&ctx, stack, dtype);
    if (!__Pyx_BufFmt_CheckString(&ctx, buf->format)) goto fail;

    if ((unsigned)buf->itemsize != dtype->size) {
        PyErr_Format(PyExc_ValueError,
                     "Item size of buffer (%" PY_FORMAT_SIZE_T "d byte%s) "
                     "does not match size of '%s' (%" PY_FORMAT_SIZE_T "d byte%s)",
                     buf->itemsize,
                     (buf->itemsize > 1) ? "s" : "",
                     dtype->name,
                     dtype->size,
                     (dtype->size > 1) ? "s" : "");
        goto fail;
    }

    if (!buf->strides) {
        PyErr_SetString(PyExc_ValueError,
            "buffer does not supply strides necessary for memoryview.");
        goto fail;
    }

    for(i=0; i<ndim; i++) {
        spec = axes_specs[i];

        if (spec & __Pyx_MEMVIEW_CONTIG) {
            if (buf->strides[i] != buf->itemsize) {
                PyErr_SetString(PyExc_ValueError,
                    "Buffer and memoryview are not contiguous in the same dimension.");
                goto fail;
            }
        }

        if (spec & (__Pyx_MEMVIEW_STRIDED | __Pyx_MEMVIEW_FOLLOW)) {
            if (buf->strides[i] < buf->itemsize) {
                PyErr_SetString(PyExc_ValueError,
                    "Buffer and memoryview are not contiguous in the same dimension.");
                goto fail;
            }
        }

        if (spec & __Pyx_MEMVIEW_DIRECT) {
            if (buf->suboffsets && buf->suboffsets[i] >= 0) {
                PyErr_SetString(PyExc_ValueError,
                    "Buffer not compatible with direct access.");
                goto fail;
            }
        }

        if (spec & (__Pyx_MEMVIEW_PTR | __Pyx_MEMVIEW_FULL)) {
            if (!buf->suboffsets) {
                PyErr_SetString(PyExc_ValueError,
                    "Buffer not able to be indirectly accessed.");
                goto fail;
            }
        }

        if (spec & __Pyx_MEMVIEW_PTR) {
            if (buf->suboffsets[i] < 0) {
                PyErr_Format(PyExc_ValueError,
                    "Buffer not indirectly accessed in %d dimension, although memoryview is.", i);
                goto fail;
            }
        }
    }

    if (c_or_f_flag & __Pyx_IS_F_CONTIG) {
        stride = 1;
        for(i=0; i<ndim; i++) {
            if(stride * buf->itemsize != buf->strides[i]) {
                PyErr_SetString(PyExc_ValueError,
                    "Buffer not fortran contiguous.");
                goto fail;
            }
            stride = stride * buf->shape[i];
        }
    } else if (c_or_f_flag & __Pyx_IS_F_CONTIG) {
        for(i=ndim-1; i>-1; i--) {
            if(stride * buf->itemsize != buf->strides[i]) {
                PyErr_SetString(PyExc_ValueError,
                    "Buffer not C contiguous.");
                goto fail;
            }
            stride = stride * buf->shape[i];
        }
    }

    if(unlikely(__Pyx_init_memviewslice(memview, ndim, memviewslice) == -1)) {
        goto fail;
    }

    retval = 0;
    goto no_fail;
fail:
    __Pyx_XDECREF(memviewslice->memview);
    memviewslice->memview = 0;
    memviewslice->data = 0;
    retval = -1;

no_fail:
    __Pyx_RefNannyFinishContext();
    return retval;
}

static int __Pyx_init_memviewslice(
                struct __pyx_memoryview_obj *memview,
                int ndim,
                __Pyx_memviewslice *memviewslice) {

    __Pyx_RefNannyDeclarations
    __Pyx_RefNannySetupContext("init_memviewslice");
    int i, retval=-1;
    Py_buffer *buf = &memview->view;

    if(!buf) {
        PyErr_SetString(PyExc_ValueError,
            "buf is NULL.");
        goto fail;
    } else if(memviewslice->memview || memviewslice->data) {
        PyErr_SetString(PyExc_ValueError,
            "memviewslice is already initialized!");
        goto fail;
    }

    for(i=0; i<ndim; i++) {
        memviewslice->strides[i] = buf->strides[i];
        memviewslice->shape[i]   = buf->shape[i];
        if(buf->suboffsets) {
            memviewslice->suboffsets[i] = buf->suboffsets[i];
        } else {
            memviewslice->suboffsets[i] = -1;
        }
    }

    memviewslice->memview = memview;
    memviewslice->data = (char *)buf->buf;
    retval = 0;
    goto no_fail;

fail:
    __Pyx_XDECREF(memviewslice->memview);
    memviewslice->memview = 0;
    memviewslice->data = 0;
    retval = -1;
no_fail:
    __Pyx_RefNannyFinishContext();
    return retval;
}

static CYTHON_INLINE void __Pyx_INC_MEMVIEW({{memviewslice_name}} *memslice,
                                            int have_gil, int lineno) {
    int first_time;
    struct {{memview_struct_name}} *memview = memslice->memview;
    if (!memview) {
        char msg[50];
        snprintf(msg, 50, "memoryslice is not initialized (line %d)", lineno);
        Py_FatalError(msg);
    }

    PyThread_acquire_lock(memview->lock, 1);
    first_time = (memview->acquisition_count++ == 0);
    PyThread_release_lock(memview->lock);

    /* printf("INCREF %d: acquisition_count=%d, refcount=%d\n", lineno,
           memview->acquisition_count, memview->ob_refcnt); */

    if (first_time) {
        if (have_gil) {
            Py_INCREF((PyObject *) memview);
        } else {
            PyGILState_STATE _gilstate = PyGILState_Ensure();
            Py_INCREF((PyObject *) memview);
            PyGILState_Release(_gilstate);
        }
    }
}

static CYTHON_INLINE void __Pyx_XDEC_MEMVIEW({{memviewslice_name}} *memslice,
                                             int have_gil, int lineno) {
    int last_time;
    struct {{memview_struct_name}} *memview = memslice->memview;

    if (!memview) {
        return;
    }

    PyThread_acquire_lock(memview->lock, 1);
    last_time = (memview->acquisition_count-- == 1);
    PyThread_release_lock(memview->lock);

    /* printf("DECREF %d: acquisition_count=%d, refcount=%d\n", lineno,
           memview->acquisition_count, memview->ob_refcnt); */

    if (last_time) {
        if (have_gil) {
            Py_CLEAR(memview);
        } else {
            PyGILState_STATE _gilstate = PyGILState_Ensure();
            Py_CLEAR(memview);
            PyGILState_Release(_gilstate);
        }
        memslice->data = NULL;
    }
}

////////// MemviewSliceCopyTemplate //////////

static __Pyx_memviewslice {{copy_name}}(const __Pyx_memviewslice from_mvs) {

    __Pyx_RefNannyDeclarations
    int i;
    __Pyx_memviewslice new_mvs = {0, 0};
    struct __pyx_memoryview_obj *from_memview = from_mvs.memview;
    Py_buffer *buf = &from_memview->view;
    PyObject *shape_tuple = 0;
    PyObject *temp_int = 0;
    struct __pyx_array_obj *array_obj = 0;
    struct __pyx_memoryview_obj *memview_obj = 0;
    char *mode = "{{mode}}";

    __Pyx_RefNannySetupContext("{{copy_name}}");

    shape_tuple = PyTuple_New((Py_ssize_t)(buf->ndim));
    if(unlikely(!shape_tuple)) {
        goto fail;
    }
    __Pyx_GOTREF(shape_tuple);


    for(i=0; i<buf->ndim; i++) {
        temp_int = PyInt_FromLong(buf->shape[i]);
        if(unlikely(!temp_int)) {
            goto fail;
        } else {
            PyTuple_SET_ITEM(shape_tuple, i, temp_int);
        }
    }

    array_obj = __pyx_array_new(shape_tuple, {{sizeof_dtype}}, buf->format, mode);
    if (unlikely(!array_obj)) {
        goto fail;
    }
    __Pyx_GOTREF(array_obj);

    memview_obj = (struct __pyx_memoryview_obj *) __pyx_memoryview_new(
                                (PyObject *) array_obj, {{contig_flag}});
    if (unlikely(!memview_obj)) {
        goto fail;
    }

    /* initialize new_mvs */
    if (unlikely(-1 == __Pyx_init_memviewslice(memview_obj, buf->ndim, &new_mvs))) {
        PyErr_SetString(PyExc_RuntimeError,
            "Could not initialize new memoryviewslice object.");
        goto fail;
    }

    if (unlikely(-1 == {{copy_contents_name}}(&from_mvs, &new_mvs))) {
        /* PyErr_SetString(PyExc_RuntimeError,
            "Could not copy contents of memoryview slice."); */
        goto fail;
    }

    goto no_fail;

fail:
    __Pyx_XDECREF(new_mvs.memview); new_mvs.memview = 0;
    new_mvs.data = 0;
no_fail:
    __Pyx_XDECREF(shape_tuple); shape_tuple = 0;
    __Pyx_GOTREF(temp_int);
    __Pyx_XDECREF(temp_int); temp_int = 0;
    __Pyx_XDECREF(array_obj); array_obj = 0;
    __Pyx_RefNannyFinishContext();
    return new_mvs;

}

/////////////// MemviewSliceIndex ///////////////

static CYTHON_INLINE char *__pyx_memviewslice_index_full(char *bufp, Py_ssize_t idx, Py_ssize_t stride, Py_ssize_t suboffset) {
    bufp = bufp + idx * stride;
    if (suboffset >= 0) {
        bufp = *((char **) bufp) + suboffset;
    }
    return bufp;
}

/* The call has already done the indexing */
static CYTHON_INLINE char *__pyx_memviewslice_index_full_contig(char *bufp, Py_ssize_t suboffset) {
    if (suboffset >= 0) {
        bufp = *((char **) bufp) + suboffset;
    }
    return bufp;
}

