#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <faac.h>
#include <stddef.h>
#include <stdbool.h>

#define RAW 0
#define ADTS 1

typedef struct {
	PyObject_HEAD
	
	#define CONFIG_OFFSETOF(member) (offsetof(pyfaac3_EncoderConfig, config) + offsetof(faacEncConfiguration, member))
  
	faacEncConfiguration config;
	PyObject *channel_map_list;
	
} pyfaac3_EncoderConfig;




static struct PyMemberDef pyfaac3_EncoderConfigMembers[] = {
	{"mpeg_version", T_UINT, CONFIG_OFFSETOF(mpegVersion), 0,
		"MPEG version"},
	{"aac_object_type", T_UINT, CONFIG_OFFSETOF(aacObjectType), 0,
		"AAC object type"},
	{"allow_midside", T_UINT, CONFIG_OFFSETOF(allowMidside), 0,
		"Set to 1 to llow mid/side coding; 0 otherwise"},
	{"use_lfe", T_UINT, CONFIG_OFFSETOF(useLfe), 0,
		"Set to 1 to use one of the channels as LFE channel; 0 otherwise"},
	{"use_tns", T_UINT, CONFIG_OFFSETOF(useTns), 0,
		"Set to 1 to use Temporal Noise Shaping; 0 otherwise"},
	{"bit_rate", T_ULONG, CONFIG_OFFSETOF(bitRate), 0,
		"bitrate per channel of a AAC file"},
	{"bandwidth", T_UINT, CONFIG_OFFSETOF(bandWidth), 0,
		"AAC file frequency bandwidth; set to 0 to allow faac to estimate the bandwidth to be used as default"},
	{"quantqual", T_ULONG, CONFIG_OFFSETOF(quantqual), 0,
		"Quantizer quality"},
	{"output_format", T_UINT, CONFIG_OFFSETOF(outputFormat), 0,
		"Bitstream output format (RAW/ADTS)"},
	{"input_format", T_UINT, CONFIG_OFFSETOF(inputFormat), 0,
		"PCM Format of input data (FAAC_INPUT_16BIT/FAAC_INPUT_32BIT/FAAC_INPUT_FLOAT)"},
	{"shortctl", T_INT, CONFIG_OFFSETOF(shortctl), 0,
		"block type enforcing (SHORTCTL_NORMAL/SHORTCTL_NOSHORT/SHORTCTL_NOLONG)"},
	{"channel_map", T_OBJECT_EX, offsetof(pyfaac3_EncoderConfig, channel_map_list), READONLY,
		"Specifying the channel format in input data as a list, e.g., default: [0, 1, 2, 3, ...]; WAVE 4.0: [2, 0, 1, 3, ...]. Supporting up to 64 channels"},


	{NULL}  /* Sentinel */
};


static void pyfaac3_EncoderConfig_dealloc(pyfaac3_EncoderConfig *self);
static PyObject *pyfaac3_EncoderConfig_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int pyfaac3_EncoderConfig_init(pyfaac3_EncoderConfig *self, PyObject *args, PyObject *kwargs);

static PyTypeObject pyfaac3_EncoderConfigType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"pyfaac3.EncoderConfig",             /* tp_name */
	sizeof(pyfaac3_EncoderConfig), /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)pyfaac3_EncoderConfig_dealloc, /* tp_dealloc */
	0,                         /* tp_print */
	0,                         /* tp_getattr */
	0,                         /* tp_setattr */
	0,                         /* tp_reserved */
	0,                         /* tp_repr */
	0,                         /* tp_as_number */
	0,                         /* tp_as_sequence */
	0,                         /* tp_as_mapping */
	0,                         /* tp_hash  */
	0,                         /* tp_call */
	0,                         /* tp_str */
	0,                         /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,        /* tp_flags */
	"The configuration of pyfaac3.Encoder",           /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	0,						             /* tp_methods */
	pyfaac3_EncoderConfigMembers, /* tp_members */
	0,									       /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)pyfaac3_EncoderConfig_init, /* tp_init */
	0,                         /* tp_alloc */
  pyfaac3_EncoderConfig_new, /* tp_new */
};


 

typedef struct {
	PyObject_HEAD
	
	unsigned long sample_rate;
	unsigned int input_sample_size;
	unsigned int number_of_channels;
	unsigned long number_of_samples_per_frame;
	unsigned long max_output_bytes;
	unsigned char *output_buffer;
	char flushing_flag;

	faacEncHandle handle;
} pyfaac3_Encoder;



static PyMemberDef pyfaac3_EncoderMembers[] = {
	{"sample_rate", T_ULONG, offsetof(pyfaac3_Encoder, sample_rate), READONLY,
		"Sample rate"},
	{"input_sample_size", T_UINT, offsetof(pyfaac3_Encoder, input_sample_size), READONLY,
		"The size of a single input sample in bytes"},
	{"number_of_channels", T_UINT, offsetof(pyfaac3_Encoder, number_of_channels), READONLY,
		"Number of channels"},
	{"number_of_samples_per_frame", T_ULONG, offsetof(pyfaac3_Encoder, number_of_samples_per_frame), READONLY,
		"The number of samples consisting of a audio frame; note that the duration of a audio frame is equal to number_of_samples_per_frame / (number_of_channels * sample_rate)"},
	{"flushing_flag", T_BOOL, offsetof(pyfaac3_Encoder, flushing_flag), READONLY,
		"True if the encoder enters flushing mode; False otherwise"},

	{NULL}  /* Sentinel */
};


static int pyfaac3_Encoder_init(pyfaac3_Encoder *self, PyObject *args, PyObject *kwargs);
static void pyfaac3_Encoder_dealloc(pyfaac3_Encoder *self);
static PyObject *pyfaac3_Encoder_set_configuration(PyObject *self_as_object, PyObject *args, PyObject *kwargs);
static PyObject *pyfaac3_Encoder_get_configuration(PyObject *self_as_object, PyObject *args);
static PyObject *pyfaac3_Encoder_encode(PyObject *self_as_object, PyObject *args, PyObject *kwargs);
static PyObject *pyfaac3_Encoder_flush(PyObject *self_as_object, PyObject *args);
static PyObject *pyfaac3_Encoder_close(PyObject *self_as_object, PyObject *args);
static PyObject *pyfaac3_Encoder___enter__(PyObject *self_as_object, PyObject *args);
static PyObject *pyfaac3_Encoder___exit__(PyObject *self_as_object, PyObject *args, PyObject *kwargs);




static PyMethodDef pyfaac3_EncoderMethods[] = {
	{"set_configuration", (PyCFunction)pyfaac3_Encoder_set_configuration, METH_VARARGS | METH_KEYWORDS,
		"Updating the current configuration to the given one by calling faacEncSetConfiguration."
	},
	{"get_configuration", (PyCFunction)pyfaac3_Encoder_get_configuration, METH_NOARGS,
		"Retrieving a copy of the current configuration obtained from faacEncGetCurrentConfiguration."
	},
	{"encode", (PyCFunction)pyfaac3_Encoder_encode, METH_VARARGS | METH_KEYWORDS,
		"Encoding the input pcm by calling faacEncEncode."
	},
	{"flush", (PyCFunction)pyfaac3_Encoder_flush, METH_NOARGS,
		"Flushing the audio frames cached inside the encoder for encoding; note that once flushing starts, the encoder can not process any more new audio data."
	},
	{"close", (PyCFunction)pyfaac3_Encoder_close, METH_NOARGS,
		"Closing the encoder by calling faacEncClose."
	},
	{"__enter__", (PyCFunction)pyfaac3_Encoder___enter__, METH_NOARGS,
		"Implementing the magic method \" __enter__\" to support the use of the \"with\" statement."
	},
	{"__exit__", (PyCFunction)pyfaac3_Encoder___exit__, METH_VARARGS | METH_KEYWORDS,
		"Implementing the magic method \" __exit__\" to support the use of the \"with\" statement."
	},


	{NULL}  /* Sentinel */
};



#if 0
static PyObject *pyfaac3_Encoder_get_frame_size(pyfaac3_Encoder *self, void *closure);

static PyGetSetDef pyfaac3_EncoderGetSets[] = {
	{"frame_size",
		(getter)pyfaac3_Encoder_get_frame_size, NULL,
		"The size of an audio frame in bytes, which is equal to (input_sample_size * number_of_samples_per_frame)",
		NULL},
	{NULL}  /* Sentinel */
};
#endif





static PyTypeObject pyfaac3_EncoderType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"pyfaac3.Encoder",             /* tp_name */
	sizeof(pyfaac3_Encoder), /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)pyfaac3_Encoder_dealloc, /* tp_dealloc */
	0,                         /* tp_print */
	0,                         /* tp_getattr */
	0,                         /* tp_setattr */
	0,                         /* tp_reserved */
	0,                         /* tp_repr */
	0,                         /* tp_as_number */
	0,                         /* tp_as_sequence */
	0,                         /* tp_as_mapping */
	0,                         /* tp_hash  */
	0,                         /* tp_call */
	0,                         /* tp_str */
	0,                         /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,        /* tp_flags */
	"An AAC encoder powered by faac",           /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	pyfaac3_EncoderMethods,		 /* tp_methods */
	pyfaac3_EncoderMembers,    /* tp_members */
	0,												 /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)pyfaac3_Encoder_init,      /* tp_init */
};




static void
pyfaac3_EncoderConfig_dealloc(pyfaac3_EncoderConfig *self)
{
	PyObject *to_be_delete = NULL;
	if(self->channel_map_list)
	{
		to_be_delete = self->channel_map_list;
		self->channel_map_list = NULL;
		Py_DECREF(to_be_delete);
	}

	Py_TYPE(self)->tp_free((PyObject*)self);
}


static int
pyfaac3_EncoderConfig_init(pyfaac3_EncoderConfig *self, PyObject *args, PyObject *kwargs)
{

	static char *kwlist[] = {
		"encoder", 
		NULL
	};

	pyfaac3_Encoder *encoder = NULL;
	Py_ssize_t channel_index = 0;
	PyObject * list_item = NULL;

	if(!PyArg_ParseTupleAndKeywords(args, kwargs,
																		"O", kwlist, &encoder))
		return -1;
	

	self->config = *faacEncGetCurrentConfiguration(encoder->handle);

	for(channel_index = 0; channel_index < sizeof(self->config.channel_map)/sizeof(self->config.channel_map[0]); ++channel_index)
	{
		list_item = PyLong_FromLong(self->config.channel_map[channel_index]);
		if(list_item == NULL || PyList_SetItem(self->channel_map_list, channel_index, list_item) < 0)
		{
			return -1;
		}
	}
	
	return 0;
}

static PyObject *
pyfaac3_EncoderConfig_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Py_ssize_t number_of_channels = 0;
	pyfaac3_EncoderConfig *self = (pyfaac3_EncoderConfig *)type->tp_alloc(type, 0);

	if (self != NULL) {

		memset(&self->config, 0, sizeof(self->config));
		number_of_channels = sizeof(self->config.channel_map)/sizeof(self->config.channel_map[0]);
		self->channel_map_list = PyList_New(number_of_channels);

		if(self->channel_map_list == NULL)
		{
			Py_DECREF(self);
			return NULL;
		}

	}

	return (PyObject *)self;
}


static void
clean_up_encoder(pyfaac3_Encoder *self)
{
	if(self->output_buffer)
	{
		free(self->output_buffer);
		self->output_buffer = NULL;
	}
	
	if(self->handle)
	{
		faacEncClose(self->handle);
		self->handle = NULL;
	}
		

}


static int
update_sample_information(pyfaac3_Encoder *self)
{

	faacEncConfigurationPtr faac_config = faacEncGetCurrentConfiguration(self->handle);
	if(faac_config == NULL)
	{
		PyErr_Format(PyExc_RuntimeError, "No config is present for this encoder");
		return -1;
	}

	switch(faac_config->inputFormat)
	{
		case FAAC_INPUT_16BIT:
			self->input_sample_size = 2;
			break;
		case FAAC_INPUT_32BIT:
		case FAAC_INPUT_FLOAT:
			self->input_sample_size = 4;
			break;
		default:
			PyErr_Format(PyExc_ValueError, "Unspported input format: %u", faac_config->inputFormat);
			return -1;
			
	}
		

	return 0;
}

/*
static PyObject *
pyfaac3_Encoder_get_frame_size(pyfaac3_Encoder *self, void *closure)
{
		    
	unsigned long frame_size = self->input_sample_size * self->number_of_samples_per_frame;
	
	return Py_BuildValue("k", frame_size);

}
*/

static int
pyfaac3_Encoder_init(pyfaac3_Encoder *self, PyObject *args, PyObject *kwargs)
{

	static char *kwlist[] = {
		"sample_rate", 
		"number_of_channels", 
		NULL
	};


	unsigned long sample_rate = 0;
	unsigned int number_of_channels = 0;

	if(!PyArg_ParseTupleAndKeywords(args, kwargs,
																		"kI", kwlist, 
																		&sample_rate, 
																		&number_of_channels))
		return -1;


	clean_up_encoder(self);

	self->handle = faacEncOpen(sample_rate, number_of_channels, &self->number_of_samples_per_frame, &self->max_output_bytes);
  if (!self->handle) {
		PyErr_Format(PyExc_RuntimeError, "Can not create an AAC encoder with sample_rate = %lu, number_of_channels = %u", sample_rate, number_of_channels);
    //PyErr_SetString(PyExc_RuntimeError, "Can not open faac encoder");
		return -1;
  }

	self->sample_rate = sample_rate;
	self->number_of_channels = number_of_channels;

	self->output_buffer = malloc(self->max_output_bytes);
	
	if(self->output_buffer == NULL)
	{

		PyErr_Format(PyExc_RuntimeError, "Can not allocate a buffer of a size %lu for storing encoded data", self->max_output_bytes);
		return -1;
	}

	if(update_sample_information(self) < 0)
		return -1;
	
	return 0;
}

static void
pyfaac3_Encoder_dealloc(pyfaac3_Encoder *self)
{
	clean_up_encoder(self);	

	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
pyfaac3_Encoder_set_configuration(PyObject *self_as_object, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = {
		"config", 
		NULL
	};

	pyfaac3_EncoderConfig *encoder_config = NULL;
	pyfaac3_Encoder *self = (pyfaac3_Encoder *)self_as_object;
	Py_ssize_t channel_index = 0, number_of_channels = 0, max_number_of_channels = 0;
	
	if(!PyArg_ParseTupleAndKeywords(args, kwargs,
																		"O", kwlist, 
																		&encoder_config))

	{
		//PyErr_Format(PyExc_RuntimeError, "Can not create an AAC encoder with sample_rate = %lu, channels = %u", self->sample_rate, self->channels)
		return NULL;
	}

	if(Py_TYPE(encoder_config) != &pyfaac3_EncoderConfigType)
	{
		PyErr_Format(PyExc_ValueError, "The input argument should be an instance of pyfaac.EncoderConfig");
		return NULL;
	}

	number_of_channels = PyList_GET_SIZE(encoder_config->channel_map_list);
	max_number_of_channels = sizeof(encoder_config->config.channel_map)/sizeof(encoder_config->config.channel_map[0]);
	if(number_of_channels > max_number_of_channels)
		number_of_channels = max_number_of_channels;

	for(channel_index = 0; channel_index < number_of_channels; ++channel_index)
	{
		encoder_config->config.channel_map[channel_index] = PyLong_AsLong(PyList_GET_ITEM(encoder_config->channel_map_list, channel_index));
		if(PyErr_Occurred())
			return NULL;

	}


  if (!faacEncSetConfiguration(self->handle, &encoder_config->config)) {
		PyErr_Format(PyExc_RuntimeError, "The call to faacEncSetConfiguration() failed; the given configuration might contain invalid settings");
		return NULL;
  }

	if(update_sample_information(self) < 0)
		return NULL;

	Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *
pyfaac3_Encoder_get_configuration(PyObject *self_as_object, PyObject *args)
{

	PyObject *config_init_args = Py_BuildValue("(O)", self_as_object);

	if(config_init_args == NULL)
	{
		return NULL;
	}


	return PyObject_CallObject((PyObject *)&pyfaac3_EncoderConfigType, config_init_args);

}


static PyObject *
pyfaac3_Encoder_encode(PyObject *self_as_object, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = {
		"raw", 
		NULL
	};

	pyfaac3_Encoder *self = (pyfaac3_Encoder *)self_as_object;
	Py_ssize_t number_of_bytes_written = 0, data_size = 0;
	unsigned int number_of_input_samples = 0;
	Py_buffer raw;
	PyObject *output = NULL;
	void *silence_buffer = NULL, *data_buffer = NULL;

	if(self->flushing_flag)
	{
		PyErr_Format(PyExc_RuntimeError, "Can not accept audio data any more once the encoder starts flushing ");
		return NULL;
	}
	

	if(!PyArg_ParseTupleAndKeywords(args, kwargs,
																		"y*", kwlist, 
																		&raw))

	{
		return NULL;
	}

	if(raw.len == 0)
	{
		data_size = self->input_sample_size * self->number_of_channels;
		silence_buffer = malloc(data_size);
		memset(silence_buffer, 0, data_size);
		data_buffer = silence_buffer;
	}
	else
	{
		data_size = raw.len;
		data_buffer = raw.buf;
	}

	do
	{
		number_of_input_samples = data_size / self->input_sample_size;
		if(number_of_input_samples > self->number_of_samples_per_frame)
		{
			PyErr_Format(PyExc_ValueError, "Too many input samples has been given (%u > %lu).", number_of_input_samples, self->number_of_samples_per_frame);
			break;
		}


		number_of_bytes_written = faacEncEncode(self->handle, data_buffer, number_of_input_samples, self->output_buffer, self->max_output_bytes);
		output = Py_BuildValue("y#", self->output_buffer, number_of_bytes_written);

	}while(false);

	if(silence_buffer)
	{
		free(silence_buffer);
		silence_buffer = NULL;
	}

	PyBuffer_Release(&raw);

	return output;
}


static PyObject *
pyfaac3_Encoder_flush(PyObject *self_as_object, PyObject *args)
{

	pyfaac3_Encoder *self = (pyfaac3_Encoder *)self_as_object;
	Py_ssize_t number_of_bytes_written = 0;
	PyObject *output = NULL;

	number_of_bytes_written = faacEncEncode(self->handle, NULL, 0, self->output_buffer, self->max_output_bytes);
	self->flushing_flag = 1;
	output = Py_BuildValue("y#", self->output_buffer, number_of_bytes_written);

	return output;
}


static PyObject *
pyfaac3_Encoder_close(PyObject *self_as_object, PyObject *args)
{

	pyfaac3_Encoder *self = (pyfaac3_Encoder *)self_as_object;
	clean_up_encoder(self);	

	Py_INCREF(Py_None);
  return Py_None;

}

static PyObject *
pyfaac3_Encoder___enter__(PyObject *self_as_object, PyObject *args)
{
  return self_as_object;
}

static PyObject *
pyfaac3_Encoder___exit__(PyObject *self_as_object, PyObject *args, PyObject *kwargs)
{
	//even this magic function passes in several input arguments, as they are not used in this particular implementation, no argument parsing is conducted
	pyfaac3_Encoder *self = (pyfaac3_Encoder *)self_as_object;
	clean_up_encoder(self);	

	Py_INCREF(Py_False);
  return Py_False;

}










static PyModuleDef pyfaac3Module = {
	PyModuleDef_HEAD_INIT,
	"pyfaac3",
	"An extention module providing access to the native library \"faac\" from a python3 environment.",
	-1,
	NULL, NULL, NULL, NULL, NULL
};



PyMODINIT_FUNC
PyInit_pyfaac3(void) 
{

	pyfaac3_EncoderType.tp_new = PyType_GenericNew;
	if (PyType_Ready(&pyfaac3_EncoderType) < 0)
		return NULL;

	if (PyType_Ready(&pyfaac3_EncoderConfigType) < 0)
		return NULL;

	PyObject *m = PyModule_Create(&pyfaac3Module);
	if (m == NULL)
		return NULL;

	do
	{
		if(PyModule_AddIntMacro(m, MPEG2) < 0 
			|| PyModule_AddIntMacro(m, MPEG4) < 0
			|| PyModule_AddIntMacro(m, MAIN) < 0
			|| PyModule_AddIntMacro(m, LOW) < 0
			//Note: SSR seems to be not supported by faac 
			|| PyModule_AddIntMacro(m, SSR) < 0
			|| PyModule_AddIntMacro(m, LTP) < 0
			|| PyModule_AddIntMacro(m, RAW) < 0
			|| PyModule_AddIntMacro(m, ADTS) < 0
			|| PyModule_AddIntMacro(m, FAAC_INPUT_16BIT) < 0
			//Note: FAAC_INPUT_24BIT seems to be not supported by faac 
			|| PyModule_AddIntMacro(m, FAAC_INPUT_24BIT) < 0
			|| PyModule_AddIntMacro(m, FAAC_INPUT_32BIT) < 0
			|| PyModule_AddIntMacro(m, FAAC_INPUT_FLOAT) < 0
			|| PyModule_AddIntMacro(m, SHORTCTL_NORMAL) < 0
			|| PyModule_AddIntMacro(m, SHORTCTL_NOSHORT) < 0
			|| PyModule_AddIntMacro(m, SHORTCTL_NOLONG) < 0
			
			)
			break;




		Py_INCREF(&pyfaac3_EncoderType);
		if(PyModule_AddObject(m, "Encoder", (PyObject *)&pyfaac3_EncoderType) < 0)
		{
			Py_DECREF(&pyfaac3_EncoderType);
			break;
		}
		Py_INCREF(&pyfaac3_EncoderConfigType);
		if(PyModule_AddObject(m, "EncoderConfig", (PyObject *)&pyfaac3_EncoderConfigType) < 0)
		{
			Py_DECREF(&pyfaac3_EncoderConfigType);
			break;
		}

		return m;
	}while(false);

	Py_DECREF(m);
	return NULL;
}

