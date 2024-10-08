FROM node:22 AS client-builder
WORKDIR /client
COPY client/package.json client/yarn.lock ./
RUN yarn install
COPY client/ ./
RUN yarn run build

ARG BUILDPLATFORM
FROM --platform=${BUILDPLATFORM} python:3.9-slim
WORKDIR /server
COPY server/requirements.txt .
RUN apt-get update && apt-get install -y \
  curl \
  build-essential \
  gcc
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"
RUN pip install --no-cache-dir --upgrade pip \
  && pip install --no-cache-dir -r requirements.txt
COPY server/ .
RUN mkdir -p static
COPY --from=client-builder /client/dist /server/static
CMD ["fastapi", "run", "main.py", "--port", "80"]