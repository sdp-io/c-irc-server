import asyncio
from draw_graph import draw_graph
from time import perf_counter

N_LIST = [100, 500, 1000, 2000, 3000, 5000, 7500, 10000]

LOCALHOST = "127.0.0.1"
PORT = 9034
CHANNEL = "#test"
TEST_MESSAGE = "test message"

PRIVMSG_TEMPLATE = f"PRIVMSG {CHANNEL} :{TEST_MESSAGE}\r\n".encode()


stream_lock = asyncio.Lock()


async def recv_message(reader):
    # If message is never received, program will not terminate
    await reader.readuntil(PRIVMSG_TEMPLATE)


async def send_message(writer):
    writer.write(PRIVMSG_TEMPLATE)
    await writer.drain()


async def send_ping(reader, writer, message):
    writer.write(f"PING :{message}\r\n".encode("utf-8"))
    await writer.drain()

    async with stream_lock:
        await reader.readuntil("\r\n".encode("utf-8"))


async def join_channel(reader, writer, channel_name):
    writer.write(f"JOIN {channel_name}\r\n".encode("utf-8"))
    await writer.drain()

    # Read lines until ENDNAMES reply is received to empty read buffer
    while True:
        line = await reader.readline()
        decoded_line = line.decode()

        if "366" in decoded_line or "ENDNAMES" in decoded_line:
            break


async def create_user(address, port, user_nick):
    # Increase reader buffer size as the NAMES reply will exceed the default 64 KiB
    reader, writer = await asyncio.open_connection(address, port, limit=1024 * 1024)

    writer.write(f"NICK {user_nick}\r\n".encode("utf-8"))
    await writer.drain()

    writer.write(f"USER {user_nick} 0 * :test user {user_nick}\r\n".encode("utf-8"))
    await writer.drain()

    # Read lines until End of MOTD reply to empty read buffer
    while True:
        line = await reader.readline()
        decoded_line = line.decode()

        if "376" in decoded_line or "End of MOTD" in decoded_line:
            break

    return [reader, writer]


async def main():
    benchmarks = []

    for N in N_LIST:
        tasks = []

        # Context manager for automated tests, create N number of users
        async with asyncio.TaskGroup() as tg:
            for i in range(N):
                current_nick = f"User_{N}_{i}"
                task = tg.create_task(create_user(LOCALHOST, PORT, current_nick))
                tasks.append(task)
                await asyncio.sleep(
                    0.001
                )  # Buffer to prevent kernel from dropping packets

        users = [task.result() for task in tasks]

        # Join N number of users to specified channel
        async with asyncio.TaskGroup() as tg:
            for i in range(N):
                task = tg.create_task(join_channel(users[i][0], users[i][1], CHANNEL))

        # # Have first user in the list of users message the channel
        # send_task = asyncio.create_task(send_message(users[0][1]))
        ping_tasks = []

        await asyncio.sleep(1)

        # Since sender is the first user in the list, start counting from 1
        for i in range(N):
            ping_tasks.append(
                asyncio.create_task(
                    send_ping(users[-1][0], users[-1][1], f"User_{N}_{i}")
                )
            )

        start = perf_counter()

        await asyncio.gather(*ping_tasks)

        end = perf_counter()
        elapsed_ms = (end - start) * 1000

        benchmarks.append(elapsed_ms)
        print(f"Elapsed: {elapsed_ms:.2f} ms")

        for i in range(N):
            users[i][1].write(b"QUIT :Benchmark over\r\n")
            users[i][1].close()
            await users[i][1].wait_closed()
            await asyncio.sleep(
                0.0001
            )  # Buffer to prevent kernel from dropping packets
            # NOTE: As every user is quitting simultaneously the QUIT relay is O(n^2)
            # so it must be delayed to prevent flooding

        if N > 3000:
            # Sleep to give server time to process QUIT messages
            await asyncio.sleep(1)

    draw_graph(N_LIST, benchmarks)


if __name__ == "__main__":
    asyncio.run(main())
