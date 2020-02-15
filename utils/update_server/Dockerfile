FROM node:alpine


# Add user and make it sudoer

ARG user=node

EXPOSE 3001

RUN mkdir -p /home/node/app
RUN mkdir -p /home/node/app/certs

WORKDIR /home/node/app

# Install app dependencies
# A wildcard is used to ensure both package.json AND package-lock.json are copied
# where available (npm@5+)
COPY package*.json ./
COPY config.js ./
COPY ./src ./
COPY ./certs ./certs


RUN chown -Rh $user:$user ./

USER $user


RUN npm install


# If you are building your code for production
# RUN npm ci --only=production



CMD [ "node", "main.js" ]