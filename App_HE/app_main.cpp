
#include <cstdio>


extern "C"
{
#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */
#include "config.h"

}


#include "app_config.hpp"
#include "cstream_node.h"
#include "appa_params.h"
#include "appb_params.h"

#include "scheduler_appa.h"
#include "scheduler_appb.h"

#include "EventQueue.hpp"
#include "StreamNode.hpp"

#include "stream_runtime_init.hpp"

#include "rtos_events.hpp"

extern "C" {
#include "container.h"
#include "network.h"

extern int app_main(void);
}

#include "init_drv_src.hpp"

#define NB_MAX_EVENTS 20
#define NB_MAX_BUFS 20

using namespace arm_cmsis_stream;



static arm_cmsis_stream::EventQueue *queue = nullptr;

// Event to the interrupt thread
#define INTERRUPT_THREAD_PRIORITY osPriorityNormal
osEventFlagsId_t cg_interruptEvent;
osThreadId_t tid_interrupts = nullptr;

static const osThreadAttr_t interrupt_thread_attr = {
        .stack_size = 4096,
        .priority = INTERRUPT_THREAD_PRIORITY,
    };



using namespace arm_cmsis_stream;

// Number of applications/networks available in this demo
#define NB_APPS 2
// 0 : KWS
// 1 : Camera
static int currentNetwork = 0;

#define SWITCH_EVENT (1 << 0)

/**
 * @brief Array of stream execution contexts, one per network.
 */
static stream_execution_context_t contexts[NB_APPS];

/**
 * @brief Parameters for appa and appb networks.
 * By convention, each parameter structs starts with a hardwareParams member
 * named 'hw_' that holds hardware connection parameters.
 */
static hardwareParams *params[NB_APPS];

#if defined(HAS_BUTTON)
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (NB_APPS == 1) {
		// If there is only one network, we don't need to post events to switch between
		// networks
		return;
	}
	osEventFlagsSet(cg_interruptEvent, SWITCH_EVENT);
	CMSISTREAM_LOG_DBG("Posted SWITCH_EVENT, old events=0x%08x\n", old);
}
#endif


// Translate interrupt events into CMSIS Stream events
void interrupt_thread_function(void *)
{
	printf("Started interrupt thread\n");

	for (;;) {
		uint32_t res = osEventFlagsWait(cg_interruptEvent, SWITCH_EVENT, osFlagsWaitAny, osWaitForever);
		if ((res & SWITCH_EVENT) != 0) {
			CMSISSTREAM_LOG_DBG("Received Switching network event\n");
			currentNetwork = (currentNetwork + 1) % NB_APPS;
			LOG_DBG("Switching to network %d\n", currentNetwork);
			stream_pause_current_scheduler();
			stream_resume_scheduler(&contexts[currentNetwork]);
			CMSISSTREAM_LOG_DBG("Context switch done\n");
		}
	}
	printf("Interrupt thread ended\n");
}

/*

 Pause and resume all nodes in a given graph.
 Those are not defined in the CMSIS STream Zephyr module because the
 CStreamNode interface can be extended by the application and support new
 interfaces.

 To work with the CMSIS Stream Zephyr module, two interfaces have been defined
 in cstream_node.h and the context_switch_intf is used in the
 pause / resume functions.

 Those functions are called by the CMSIS Stream Zephyr module which does not
 have any visibility on the application specific CStreamNode structure.

*/
static void pause_scheduler_app(const stream_execution_context_t *context)
{
	for (int32_t nodeid = 0; nodeid < (int32_t)context->nb_identified_nodes; nodeid++) {
		CStreamNode *cnode = static_cast<CStreamNode *>(context->get_node_by_id(nodeid));
		if (cnode != nullptr) {
			// If the node implements the context switch interface, pause it
			if ((cnode->obj != nullptr) && (cnode->context_switch_intf != nullptr)) {
				cnode->context_switch_intf->pause(cnode->obj);
			}
		}
	}
}

static void resume_scheduler_app(const stream_execution_context_t *context)
{
	for (int32_t nodeid = 0; nodeid < (int32_t)context->nb_identified_nodes; nodeid++) {
		CStreamNode *cnode = static_cast<CStreamNode *>(context->get_node_by_id(nodeid));

		if (cnode != nullptr) {
			// If the node implements the context switching interface, resume it
			if ((cnode->obj != nullptr) && (cnode->context_switch_intf != nullptr)) {
				cnode->context_switch_intf->resume(cnode->obj);
			}
		}
	}
}

/*

The get_scheduler_node functions cannot be used directly by the
CMSIS Stream Zephyr module necause they return an application dependent
CStreamNode* pointer.
So we need to wrap them to return a void* pointer.

*/
static void *get_appa_node(int32_t nodeID)
{
	return static_cast<void *>(get_scheduler_appa_node(nodeID));
}

static void *get_appb_node(int32_t nodeID)
{
	return static_cast<void *>(get_scheduler_appb_node(nodeID));
}



#if defined(HAS_BUTTON)
static int config_button()
{
	if (!gpio_is_ready_dt(&button)) {
		printk("Error: button device %s is not ready\n", button.port->name);
		return -1;
	}

	int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n", ret, button.port->name,
		       button.pin);
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n", ret,
		       button.port->name, button.pin);
		return ret;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	return (0);
}
#endif


int app_main(void)
{

    osKernelInitialize();


    int err;
	EventQueue *queue_app[NB_APPS];

	CMSISSTREAM_LOG_ERR("Starting main\n");

	/*
	Configure hardware audio source and display
	Initialize memory slab for audio buffers

	Instead of hardcoding initialization of HW peripheral here,
	one could extend the ContextSwitch interface to query what
	is the need of a node.
	Then, we could ask all nodes in each graph what they need.
	And we would only initialize the HW peripherals needed by the nodes
	in any graph.

	*/

#if defined(HAS_AUDIO)
	k_mem_slab *mem_slab = nullptr;
	const void *i2s_mic = init_audio_source(&mem_slab);

	if (i2s_mic == nullptr) {
		LOG_ERR("Error initializing audio source\n");
		goto error;
	}
#else
	const void *i2s_mic = nullptr;
#endif

// Config button


// Config display

#if defined(EXTERNAL_NETWORK)
    err = setup_flash();
    if (err)
    {
        LOG_ERR("Error initializing flash\n");
        goto error;
    }
	err = validate_container_description("c4b4312989d06dd45d92da7af0913afd");
	if (err) 
	{
		LOG_ERR("Invalid container description in external flash\n");
		goto error;
	} 
	else 
	{
		LOG_INF("Container description in external flash validated\n");
	}
#endif

	/**
	 * @brief Network parameter initialization
	 * Most settings could come from a YAML file. The YAML
	 * file could be used to generate the appa_params.c and
	 * appb_params.c files.
	 *
	 * The parameters related to hardware connections are set here
	 * in main.cpp. They are common to all graphs so a generic loop
	 * can be used. The convention has been imposed that each
	 * prameter structure for each graph starts with a hardwareParams
	 * member named 'hw_'.
	 *
	 * The TensorFlow Lite model pointer and size are also set here
	 * in main.cpp for the appa graph.
	 * It would probaby be better to put this in a YAML file too
	 * (name of variable containing the model pointer and size).
	 * Or have another mechanism to select the model from yaml.
	 * Here we have to hardcode the initialization of the node.
	 * So it is not generic. If the network contains multiple TFLite
	 * nodes with different models, this approach would not work.
	 * We would need to change the initialization.
	 * It would be better if the initialization in this
	 * file could work with any network and don't have to be changed.
	 * If is possible but require to define some conventions
	 */

	/*

	Init settings for appa scheduler

	*/
#if 1
    appaParams.audioSource.value = 0;
	params[0] = reinterpret_cast<hardwareParams *>(&appaParams);

	/*
	
	Init settings for appb scheduler
	
	*/
    appbParams.audioSource.value = 0;
	params[1] = reinterpret_cast<hardwareParams *>(&appbParams);

#else
	appaParams.audioSource.value = 0;
	params[0] = reinterpret_cast<hardwareParams *>(&appaParams);
#endif

	/**
	 * @brief Populate hardwareParams for each network
	 * by setting the i2s_mic and mem_slab members.
	 */
	for (int network = 0; network < NB_APPS; network++) {
		params[network]->i2s_mic = nullptr;
	}

	err = stream_init_memory();
	if (err != 0) {
		CMSISSTREAM_LOG_ERR("Error initializing stream\n");
		goto error;
	}

	/* Event queue init */
	for (int network = 0; network < NB_APPS; network++) {
		queue_app[network] = stream_new_event_queue();

		if (queue_app[network] == nullptr) {
			CMSISSTREAM_LOG_ERR("Can't create CMSIS Stream Event Queue for network %d\n", network);
			goto error;
		}
	}

	// Init nodes
#if 1
	err = init_scheduler_appa(queue_app[0],&appaParams);
	if (err != CG_SUCCESS) {
		CMSISSTREAM_LOG_ERR("Error: Failure during scheduler initialization for appa.\n");
		goto error;
	}

	err = init_scheduler_appb(queue_app[1],&appbParams);
	if (err != CG_SUCCESS) {
		CMSISSTREAM_LOG_ERR("Error: Failure during scheduler initialization for appb.\n");
		goto error;
	}


#else
	err = init_scheduler_appc(queue_app[0], &appcParams);
	if (err != CG_SUCCESS) {
		CMSISTREAM_LOG_ERR("Error: Failure during scheduler initialization for appc.\n");
		goto error;
	}
#endif

	cg_interruptEvent= osEventFlagsNew(NULL);

	/* Thread inits */
    tid_interrupts = osThreadNew(interrupt_thread_function, NULL, &interrupt_thread_attr);
	

	printf("Initialize contexts");

#if 1
	contexts[0] = {
		.dataflow_scheduler = scheduler_appa,
		.reset_fifos = reset_fifos_scheduler_appa,
		.pause_all_nodes = pause_scheduler_app,
		.resume_all_nodes = resume_scheduler_app,
		.get_node_by_id = get_appa_node,
		.evtQueue = queue_app[0],
		.nb_identified_nodes = STREAM_APPA_NB_IDENTIFIED_NODES,
		.scheduler_length = STREAM_APPA_SCHED_LEN
	};

	contexts[1] = {
		.dataflow_scheduler = scheduler_appb,
		.reset_fifos = reset_fifos_scheduler_appb,
		.pause_all_nodes = pause_scheduler_app,
		.resume_all_nodes = resume_scheduler_app,
		.get_node_by_id = get_appb_node,
		.evtQueue = queue_app[1],
		.nb_identified_nodes = STREAM_APPB_NB_IDENTIFIED_NODES,
		.scheduler_length = STREAM_APPB_SCHED_LEN
	};

	

#else

	contexts[0] = {.dataflow_scheduler = scheduler_appa,
		       .reset_fifos = reset_fifos_scheduler_appa,
		       .pause_all_nodes = pause_scheduler_app,
		       .resume_all_nodes = resume_scheduler_app,
		       .get_node_by_id = get_appa_node,
		       .evtQueue = queue_app[0],
		       .nb_identified_nodes = STREAM_APPA_NB_IDENTIFIED_NODES,
		       .scheduler_length = STREAM_APPA_SCHED_LEN};
#endif

	printf("Try to start first network");

	/*

	resume is called (like in a context switch) to allow
	the nodes to publish events before starting if they need to.
	event queue accepts events when resume is called.

	We cannot do it in init function of the node since in init we
	do not know if the graph is going to be run or if it is paused.

	If there is a need to distinguish start from resume then
	it has to be done in each node with a state variable.

	*/
	resume_scheduler_app(&contexts[currentNetwork]);
	stream_start_threads(&contexts[currentNetwork]);

    osKernelStart();

    stream_wait_for_threads_end();

	free_scheduler_appa();
	free_scheduler_appb();

	for (int network = 0; network < NB_APPS; network++) {
		delete queue_app[network];
	}

	stream_free_memory();

error:
	CMSISSTREAM_LOG_ERR("Fatal error in main, stopping execution\n");
	for(;;)
    {

    }
	return 0;
}
