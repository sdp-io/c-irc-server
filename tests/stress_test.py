import asyncio


async def join_channel(reader, writer, channel_name):
    writer.write(f"JOIN {channel_name}\r\n".encode("utf-8"))
    await writer.drain()

    join_reply = await reader.readuntil("\r\n".encode("utf-8"))
    print(f"Received: {join_reply.decode()!r}")

    names_reply = await reader.readuntil("\r\n".encode("utf-8"))
    print(f"Received: {names_reply.decode()!r}")

    end_of_names_reply = await reader.readuntil("\r\n".encode("utf-8"))
    print(f"Received: {end_of_names_reply.decode()!r}")


async def create_user(address, port, user_nick):
    reader, writer = await asyncio.open_connection(address, port)

    writer.write(f"NICK {user_nick}\r\n".encode("utf-8"))
    await writer.drain()

    writer.write(f"USER {user_nick} 0 * :test user {user_nick}\r\n".encode("utf-8"))
    await writer.drain()

    # TODO: Handle new MOTD and LUSERS replies on new user registration

    # NOTE: Utilizing readuntil instead of readline to catch errors when the expected
    # numeric reply is not received.
    welcome_reply = await reader.readuntil("\r\n".encode("utf-8"))
    print(f"Received: {welcome_reply.decode()!r}")

    return [reader, writer]


async def main():
    localhost = "127.0.0.1"
    port = 9034
    tasks = []
    n = 5

    # Context manager for automated tests
    async with asyncio.TaskGroup() as tg:
        for i in range(n):
            current_nick = "user_" + str(i)
            task = tg.create_task(create_user(localhost, port, current_nick))
            tasks.append(task)

    print("Completed without error!")
    print("Closing connections...")

    users = [task.result() for task in tasks]

    async with asyncio.TaskGroup() as tg:
        for i in range(n):
            task = tg.create_task(join_channel(users[i][0], users[i][1], "#test"))

    # TEST: Temporary closing of users
    for i in range(n):
        users[i][1].close()
        await users[i][1].wait_closed()


if __name__ == "__main__":
    asyncio.run(main())
