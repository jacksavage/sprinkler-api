FROM node:22 as client-builder
WORKDIR /client
COPY client/package.json client/yarn.lock ./
RUN yarn install
COPY client/ ./
RUN yarn run build

FROM --platform=linux/arm/v6 python:3.9-slim
WORKDIR /server
COPY server/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY server/ .
RUN mkdir -p static
COPY --from=client-builder /client/dist /server/static
CMD ["fastapi", "run", "main.py", "--port", "80"]