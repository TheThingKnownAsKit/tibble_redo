## Explanation
So the main thing to keep in mind is that this project uses Zenoh instead of Fast DDS for the ROS middleware. The most basic difference is that Fast DDS uses automatic discovery that often breaks over routers, through firewalls, or NAT. Fast DDS is nice for getting started because the automatic discovery requires no explicit mapping. However, it is flimsy. With our networking requirements (router and multiple computers), the node data often gets losted or grabbed by something else. Instead, we use Zenoh, which uses explicit TCP/UDP bindings for a deterministic connection that works over more complicated topology. It can connect multiple machines and handle routing. The main downside is more setup and it requires explicit mapping to be configured which can get annoying. DDS is "hope everything finds each other" and Zrnoh is "everything connects here explicitly".

Zenoh components:
1. Zenoh router (zenohd)
2. Zenoh clients (nodes)
    - This is set by just using the environment variable RMW_IMPLEMENTATION=rmw_zenoh_cpp which makes all ROS traffic go through Zenoh and is done in the container compose
3. Endpoints
    - tcp or udp

## The networking goal: allow easy switching between different networking environments that is preferably only one change and a whole new stack is working

## Environment switching

While docker composing, simply select which profile (make sure you look at .env.example and do anything required first)

- `docker compose up` is the default way to compose and assumes you are using the remote profile (no iplink router but use Zenoh)
- `docker compose --profile competition up` assumes you are wanting the full competition network stack, so yes iplink router and yes Zenoh
- `docker compose --profile wired up` assumes you are either using only one machine to develop or are using wired control (no Zenoh, Fast DDS)