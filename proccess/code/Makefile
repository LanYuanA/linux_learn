CC := gcc
p_c_mq: producer consumer
producer: producer.c
    -$(CC) -o $@ $^
cosumer: consumer.c
    -$(CC) -o $@ $^
pc_mq_clean:
    -rm ./producer ./consumer

