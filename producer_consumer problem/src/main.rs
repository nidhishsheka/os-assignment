use std::sync::{Arc, Condvar, Mutex};
use std::thread;
use std::time::Duration;

struct Buffer {
    data: Vec<i32>,
    in_pos: usize,
    out_pos: usize,
    cnt: usize,
    cap: usize,
}

impl Buffer {
    fn new(cap: usize) -> Self {
        Buffer {
            data: vec![0; cap],
            in_pos: 0,
            out_pos: 0,
            cnt: 0,
            cap,
        }
    }

    fn produce(&mut self, item: i32) {
        self.data[self.in_pos] = item;

        self.in_pos = (self.in_pos + 1) % self.cap;
        self.cnt += 1;
    }

    fn consume(&mut self) -> i32 {
        let item = self.data[self.out_pos];

        self.out_pos = (self.out_pos + 1) % self.cap;
        self.cnt -= 1;

        item
    }
}

fn main() {

    let shared = Arc::new((
        Mutex::new(Buffer::new(5)),
        Condvar::new(),
    ));

    let producer_shared = Arc::clone(&shared);
    let consumer_shared = Arc::clone(&shared);

    let producer = thread::spawn(move || {
        for item in 1..=20 {
            {
                let mut buffer = producer_shared.0.lock().unwrap();

                while buffer.cnt == buffer.cap {
                    println!(
                        "Producer: BUFFER is FULL... So, i'm waiting brother"
                    );

                    buffer = producer_shared.1.wait(buffer).unwrap();

                    println!(
                        "Producer: I just woke up , Let me just check the buffer again"
                    );
                }

                buffer.produce(item);

                println!(
                    "Producer: I produced this guy -> {}  ,   Buffer count: {}",
                    item, buffer.cnt
                );

                producer_shared.1.notify_one();
            }

            thread::sleep(Duration::from_millis(100));
        }

        println!("Producer: I'm Done.");
    });


    let consumer = thread::spawn(move || {
        for _ in 1..=20 {
            {
                let mut buffer = consumer_shared.0.lock().unwrap();

                while buffer.cnt == 0 {
                    println!(
                        "Consumer: I'm hungry, but BUFFER is EMPTY . So, i'll wait"
                    );

                    buffer = consumer_shared.1.wait(buffer).unwrap();

                    println!(
                        "Consumer: Hey, i just woke up . I'm checking the buffer again"
                    );
                }

                let item = buffer.consume();

                println!(
                    "Consumer: I just consumed {} | Buffer cnt: {} , So, that's how much is left.",
                    item, buffer.cnt
                );

                consumer_shared.1.notify_one();
            }

            thread::sleep(Duration::from_millis(150));
        }

        println!("Consumer: I'm done , what a great meal");
    });

    producer.join().unwrap();
    consumer.join().unwrap();

    println!("\nMain: Finally, Producer and Consumer have finished.");
}