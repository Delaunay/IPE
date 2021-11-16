IPE
===

IPE can be used as a synchronization point for multi-node workers.

# Build

.. code-block:: bash

    git clone --recursive github.com/IPE
    mkdir build
    cd build
    cmake ..
    make -j 8


# Pytorch & Multiprocessing


.. code-block::python

    from ipe import IPEStore

    with IPEStore(FILE_NAME, MAX_WORKER, RDV_TIMEOUT, INIT_TIMEOUT) as store:
        torch.distributed.init_process_group(
            backend='nccl', init_method=store
        )


# Low level usage

.. code-block:: cpp

   INIT_TIMEOUT = 5
   FILE_NAME = "status"
   WORKER_COUNT = 5
   RDV_TIMEOUT = 10

   // on each worker node
   ipe::init_ffa(FILE_NAME, WORKER_COUNT, INIT_TIMEOUT);
   ipe::Worker w(WORKER_ID, FILE_NAME, WORKER_COUNT);

   // Set a name for debugging/monitoring
   w.set_name(fmt::format("{}", WORKER_ID));
   w.write();

   // wait for all workers to join
   // if timeout is reached; code continues with less worker than expected
   worker_count = w.rendezvous(RDV_TIMEOUT);

   // select an active worker to use as our main process moving forward
   selected_worker_ip = w.select("123")


.. code-block: cpp

   monitor = new_monitor(FILE_NAME, WORKER_COUNT);
   monitor.status(std::cout);
