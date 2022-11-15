FROM docker.io/debian:bullseye-slim

RUN apt-get update && apt-get install -y curl build-essential cmake libssl-dev

ARG NODE_VERSION
ENV NVM_DIR="/root/.nvm"

RUN curl --silent -o- https://raw.githubusercontent.com/creationix/nvm/master/install.sh | bash \
  && . $NVM_DIR/nvm.sh \
  && nvm install $NODE_VERSION \
  && nvm alias default $NODE_VERSION \
  && nvm use default

ENV NODE_PATH $NVM_DIR/versions/node/$NODE_VERSION/lib/node_modules
ENV PATH $NVM_DIR/versions/node/$NODE_VERSION/bin:$PATH

RUN corepack enable

VOLUME /app
WORKDIR /app

CMD ["npm", "run", "prepare-prebuilds"]